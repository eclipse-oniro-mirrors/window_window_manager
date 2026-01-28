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

#include "window_manager_service.h"

#include <thread>

#include <ability_manager_client.h>
#include <cinttypes>
#include <chrono>
#include <hisysevent.h>
#include <hitrace_meter.h>
#include <ipc_skeleton.h>
#include <parameters.h>
#include <rs_iwindow_animation_controller.h>
#include "scene_board_judgement.h"
#include <system_ability_definition.h>
#include <sstream>
#include "xcollie/watchdog.h"

#include "color_parser.h"
#include "display_manager_service_inner.h"
#include "dm_common.h"
#include "drag_controller.h"
#include "memory_guard.h"
#include "minimize_app.h"
#include "permission.h"
#include "persistent_storage.h"
#include "remote_animation.h"
#include "rs_adapter.h"
#include "singleton_container.h"
#include "starting_window.h"
#include "ui/rs_ui_director.h"
#include "window_helper.h"
#include "window_inner_manager.h"
#include "window_layout_policy.h"
#include "window_manager_agent_controller.h"
#include "window_manager_hilog.h"
#include "wm_common.h"
#include "wm_math.h"

namespace OHOS {
namespace Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "WMS"};
}
WM_IMPLEMENT_SINGLE_INSTANCE(WindowManagerService)

const bool REGISTER_RESULT = SceneBoardJudgement::IsSceneBoardEnabled() ? false :
    SystemAbility::MakeAndRegisterAbility(&SingletonContainer::Get<WindowManagerService>());
const std::string BOOTEVENT_WMS_READY = "bootevent.wms.ready";

WindowManagerService::WindowManagerService() : SystemAbility(WINDOW_MANAGER_SERVICE_ID, true),
    rsInterface_(RSInterfaces::GetInstance()),
    windowShowPerformReport_(new PerformReporter("SHOW_WINDOW_TIME", {20, 35, 50}))
{
    windowRoot_ = new WindowRoot(
        [this](Event event, const sptr<IRemoteObject>& remoteObject) { OnWindowEvent(event, remoteObject); });
    inputWindowMonitor_ = new InputWindowMonitor(windowRoot_);
    windowController_ = new WindowController(windowRoot_, inputWindowMonitor_);
    dragController_ = new DragController(windowRoot_);
    windowDumper_ = new WindowDumper(windowRoot_);
    freezeDisplayController_ = new FreezeController();
    windowCommonEvent_ = std::make_shared<WindowCommonEvent>();
    startingOpen_ = system::GetParameter("persist.window.sw.enabled", "1") == "1"; // startingWin default enabled
    windowGroupMgr_ = new WindowGroupMgr(windowRoot_);
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        return;
    }
    runner_ = AppExecFwk::EventRunner::Create(name_);
    handler_ = std::make_shared<AppExecFwk::EventHandler>(runner_);
    snapshotController_ = new SnapshotController(windowRoot_, handler_);
    int ret = HiviewDFX::Watchdog::GetInstance().AddThread(name_, handler_);
    if (ret != 0) {
        WLOGFE("Add watchdog thread failed");
    }
    handler_->PostTask([]() { MemoryGuard cacheGuard; }, "WindowManagerService:cacheGuard", 0,
        AppExecFwk::EventQueue::Priority::IMMEDIATE);
    // init RSUIDirector, it will handle animation callback
    InitRSUIDirector();
}

void WindowManagerService::OnStart()
{
    WLOGI("start");
    if (!Init()) {
        WLOGFE("Init failed");
        return;
    }
    WindowInnerManager::GetInstance().Start(system::GetParameter("persist.window.holder.enable", "0") == "1");
    WindowInnerManager::GetInstance().StartWindowInfoReportLoop();
    WindowInnerManager::GetInstance().SetWindowRoot(windowRoot_);
    sptr<IDisplayChangeListener> listener = new DisplayChangeListener();
    DisplayManagerServiceInner::GetInstance().RegisterDisplayChangeListener(listener);

    sptr<IWindowInfoQueriedListener> windowInfoQueriedListener = new WindowInfoQueriedListener();
    DisplayManagerServiceInner::GetInstance().RegisterWindowInfoQueriedListener(windowInfoQueriedListener);
    system::SetParameter(BOOTEVENT_WMS_READY.c_str(), "true");
    AddSystemAbilityListener(RENDER_SERVICE);
    AddSystemAbilityListener(ABILITY_MGR_SERVICE_ID);
    AddSystemAbilityListener(COMMON_EVENT_SERVICE_ID);
    AddSystemAbilityListener(MULTIMODAL_INPUT_SERVICE_ID);
    sptr<WindowManagerService> wms = this;
    wms->IncStrongRef(nullptr);
    if (!Publish(sptr<WindowManagerService>(this))) {
        WLOGFE("Publish failed");
    }
    WLOGI("end");
}

void WindowManagerService::PostAsyncTask(Task task, const std::string& taskName, uint32_t delay)
{
    if (handler_) {
        bool ret = handler_->PostTask(task, "wms:" + taskName, delay, AppExecFwk::EventQueue::Priority::IMMEDIATE);
        if (!ret) {
            WLOGFE("EventHandler PostTask Failed");
        }
    }
}

void WindowManagerService::PostVoidSyncTask(Task task, const std::string& taskName)
{
    if (handler_) {
        bool ret = handler_->PostSyncTask(task, "wms:" + taskName, AppExecFwk::EventQueue::Priority::IMMEDIATE);
        if (!ret) {
            WLOGFE("EventHandler PostVoidSyncTask Failed");
        }
    }
}

void WindowManagerService::OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    WLOGI("systemAbilityId: %{public}d, start", systemAbilityId);
    switch (systemAbilityId) {
        case RENDER_SERVICE:
            WLOGI("RENDER_SERVICE");
            InitWithRanderServiceAdded();
            break;
        case ABILITY_MGR_SERVICE_ID:
            WLOGI("ABILITY_MGR_SERVICE_ID");
            InitWithAbilityManagerServiceAdded();
            break;
        case COMMON_EVENT_SERVICE_ID:
            WLOGI("COMMON_EVENT_SERVICE_ID");
            windowCommonEvent_->SubscriberEvent();
            break;
        case MULTIMODAL_INPUT_SERVICE_ID:
            WLOGI("MULTIMODAL_INPUT_SERVICE_ID");
            if (windowRoot_ == nullptr) {
                WLOGFE("windowRoot_ is nullptr.");
                return;
            }
            windowRoot_->NotifyMMIServiceOnline();
            break;
        default:
            WLOGFW("unhandled sysabilityId: %{public}d", systemAbilityId);
            break;
    }
    WLOGI("systemAbilityId: %{public}d, end", systemAbilityId);
}

void WindowManagerService::SetWindowInputEventConsumer()
{
    WindowInnerManager::GetInstance().SetInputEventConsumer();
}

void WindowManagerService::OnAccountSwitched(int accountId)
{
    auto task = [this, accountId]() {
        windowRoot_->RemoveSingleUserWindowNodes(accountId);
    };
    PostAsyncTask(task, "OnAccountSwitched");
    WLOGI("called");
}

void WindowManagerService::WindowVisibilityChangeCallback(std::shared_ptr<RSOcclusionData> occlusionData)
{
    WLOGI("NotifyWindowVisibilityChange: enter");
    std::weak_ptr<RSOcclusionData> weak(occlusionData);
    auto task = [this, weak]() {
        auto weakOcclusionData = weak.lock();
        if (weakOcclusionData == nullptr) {
            WLOGFE("weak occlusionData is nullptr");
            return;
        }
        windowRoot_->NotifyWindowVisibilityChange(weakOcclusionData);
    };
    PostVoidSyncTask(task, "WindowVisibilityChangeCallback");
}

void WindowManagerService::InitWithRanderServiceAdded()
{
    auto windowVisibilityChangeCb =
        [this](std::shared_ptr<RSOcclusionData> occlusionData) { this->WindowVisibilityChangeCallback(occlusionData); };
    WLOGI("RegisterWindowVisibilityChangeCallback");
    if (rsInterface_.RegisterOcclusionChangeCallback(windowVisibilityChangeCb) != WM_OK) {
        WLOGFE("RegisterWindowVisibilityChangeCallback failed");
    }
}

void WindowManagerService::InitWithAbilityManagerServiceAdded()
{
    if (snapshotController_ == nullptr) {
        snapshotController_ = new SnapshotController(windowRoot_, handler_);
    }
    WLOGI("RegisterSnapshotHandler");
    if (AAFwk::AbilityManagerClient::GetInstance()->RegisterSnapshotHandler(snapshotController_) != ERR_OK) {
        WLOGFE("RegisterSnapshotHandler failed");
    }

    if (wmsHandler_ == nullptr) {
        wmsHandler_ = new WindowManagerServiceHandler();
    }
    WLOGI("RegisterWindowManagerServiceHandler");
    bool animaEnabled = RemoteAnimation::CheckAnimationController();
    if (AAFwk::AbilityManagerClient::GetInstance()->RegisterWindowManagerServiceHandler(
        wmsHandler_, animaEnabled) != ERR_OK) {
        WLOGFE("RegisterWindowManagerServiceHandler failed");
    }
}

void WindowManagerServiceHandler::NotifyWindowTransition(
    sptr<AAFwk::AbilityTransitionInfo> from, sptr<AAFwk::AbilityTransitionInfo> to, bool& animaEnabled)
{
    sptr<WindowTransitionInfo> fromInfo = nullptr;
    sptr<WindowTransitionInfo> toInfo = nullptr;
    if (from) { // if exists, transition to window transition info
        fromInfo = new WindowTransitionInfo(from);
    }
    if (to) {
        toInfo = new WindowTransitionInfo(to);
    }
    animaEnabled = RemoteAnimation::CheckAnimationController();
    WindowManagerService::GetInstance().NotifyWindowTransition(fromInfo, toInfo, false);
}

void WindowManagerServiceHandler::NotifyAnimationAbilityDied(sptr<AAFwk::AbilityTransitionInfo> info)
{
    sptr<WindowTransitionInfo> windowTransitionInfo = new WindowTransitionInfo(info);
    WindowManagerService::GetInstance().NotifyAnimationAbilityDied(windowTransitionInfo);
}

int32_t WindowManagerServiceHandler::GetFocusWindow(sptr<IRemoteObject>& abilityToken)
{
    return static_cast<int32_t>(WindowManagerService::GetInstance().GetFocusWindowInfo(abilityToken));
}

void WindowManagerServiceHandler::StartingWindow(
    sptr<AAFwk::AbilityTransitionInfo> info, std::shared_ptr<Media::PixelMap> pixelMap)
{
    TLOGI(WmsLogTag::WMS_STARTUP_PAGE, "hot start is called");
    sptr<WindowTransitionInfo> windowInfo = new WindowTransitionInfo(info);
    WindowManagerService::GetInstance().StartingWindow(windowInfo, pixelMap, false);
}

void WindowManagerServiceHandler::StartingWindow(
    sptr<AAFwk::AbilityTransitionInfo> info, std::shared_ptr<Media::PixelMap> pixelMap, uint32_t bgColor)
{
    TLOGI(WmsLogTag::WMS_STARTUP_PAGE, "cold start is called");
    sptr<WindowTransitionInfo> windowInfo = new WindowTransitionInfo(info);
    WindowManagerService::GetInstance().StartingWindow(windowInfo, pixelMap, true, bgColor);
}

void WindowManagerServiceHandler::CancelStartingWindow(sptr<IRemoteObject> abilityToken)
{
    TLOGI(WmsLogTag::WMS_STARTUP_PAGE, "called");
    WindowManagerService::GetInstance().CancelStartingWindow(abilityToken);
}

int32_t WindowManagerServiceHandler::MoveMissionsToForeground(const std::vector<int32_t>& missionIds,
    int32_t topMissionId)
{
    TLOGD(WmsLogTag::WMS_STARTUP_PAGE, "called");
    return static_cast<int32_t>(WindowManagerService::GetInstance().MoveMissionsToForeground(missionIds, topMissionId));
}

int32_t WindowManagerServiceHandler::MoveMissionsToBackground(const std::vector<int32_t>& missionIds,
    std::vector<int32_t>& result)
{
    TLOGD(WmsLogTag::WMS_STARTUP_PAGE, "called");
    return static_cast<int32_t>(WindowManagerService::GetInstance().MoveMissionsToBackground(missionIds, result));
}

bool WindowManagerService::Init()
{
    WLOGI("Init start");
    if (WindowManagerConfig::LoadConfigXml()) {
        if (WindowManagerConfig::GetConfig().IsMap()) {
            WindowManagerConfig::DumpConfig(*WindowManagerConfig::GetConfig().mapValue_);
        }
        LoadWindowParameter();
        ConfigureWindowManagerService();
        StartingWindow::SetAnimationConfig(WindowNodeContainer::GetAnimationConfigRef());
    }
    if (PersistentStorage::HasKey("maximize_state", PersistentStorageType::MAXIMIZE_STATE)) {
        int32_t storageMode = -1;
        PersistentStorage::Get("maximize_state", storageMode, PersistentStorageType::MAXIMIZE_STATE);
        if (storageMode == static_cast<int32_t>(MaximizeMode::MODE_AVOID_SYSTEM_BAR) ||
            storageMode == static_cast<int32_t>(MaximizeMode::MODE_FULL_FILL)) {
            maximizeMode_ = static_cast<MaximizeMode>(storageMode);
        }
    }
    WindowSystemEffect::SetWindowRoot(windowRoot_);
    WLOGI("Init success");
    return true;
}

int WindowManagerService::Dump(int fd, const std::vector<std::u16string>& args)
{
    if (windowDumper_ == nullptr) {
        windowDumper_ = new WindowDumper(windowRoot_);
    }
    WLOGFI("Pid : %{public}d", IPCSkeleton::GetCallingPid());
    auto task = [this, fd, &args]() {
        return static_cast<int>(windowDumper_->Dump(fd, args));
    };
    return PostSyncTask(task, "Dump");
}

void WindowManagerService::LoadWindowParameter()
{
    const std::string multiWindowUIType = system::GetParameter("const.window.multiWindowUIType", "");
    if (multiWindowUIType == "HandsetSmartWindow") {
        systemConfig_.windowUIType_ = StartingWindow::windowUIType_ =
            WindowNodeContainer::windowUIType_ = WindowUIType::PHONE_WINDOW;
    } else if (multiWindowUIType == "FreeFormMultiWindow") {
        systemConfig_.windowUIType_ = StartingWindow::windowUIType_ =
            WindowNodeContainer::windowUIType_ = WindowUIType::PC_WINDOW;
    } else if (multiWindowUIType == "TabletSmartWindow") {
        systemConfig_.windowUIType_ = StartingWindow::windowUIType_ =
            WindowNodeContainer::windowUIType_ = WindowUIType::PAD_WINDOW;
    } else {
        WLOGFE("unknown multiWindowUIType:%{public}s.", multiWindowUIType.c_str());
    }
}

void WindowManagerService::ConfigureWindowManagerService()
{
    const auto& config = WindowManagerConfig::GetConfig();
    WindowManagerConfig::ConfigItem item = config["decor"];
    if (item.IsMap()) {
        ConfigDecor(item);
    }
    item = config["minimizeByOther"].GetProp("enable");
    if (item.IsBool()) {
        MinimizeApp::SetMinimizedByOtherConfig(item.boolValue_);
    }
    item = config["stretchable"].GetProp("enable");
    if (item.IsBool()) {
        systemConfig_.isStretchable_ = item.boolValue_;
    }
    item = config["defaultWindowMode"];
    if (item.IsInts()) {
        auto numbers = *item.intsValue_;
        if (numbers.size() == 1 &&
            (numbers[0] == static_cast<int32_t>(WindowMode::WINDOW_MODE_FULLSCREEN) ||
             numbers[0] == static_cast<int32_t>(WindowMode::WINDOW_MODE_FLOATING))) {
            systemConfig_.defaultWindowMode_ = static_cast<WindowMode>(static_cast<uint32_t>(numbers[0]));
            StartingWindow::SetDefaultWindowMode(systemConfig_.defaultWindowMode_);
        }
    }
    item = config["dragFrameGravity"];
    if (item.IsInts()) {
        auto numbers = *item.intsValue_;
        if (numbers.size() == 1
            && (numbers[0] == static_cast<int32_t>(Gravity::RESIZE)
            || numbers[0] == static_cast<int32_t>(Gravity::TOP_LEFT))) {
            windowController_->SetDragFrameGravity(static_cast<int32_t>(numbers[0]));
        }
    }
    item = config["remoteAnimation"].GetProp("enable");
    if (item.IsBool()) {
        RemoteAnimation::isRemoteAnimationEnable_ = item.boolValue_;
    }
    item = config["maxAppWindowNumber"];
    if (item.IsInts()) {
        auto numbers = *item.intsValue_;
        if (numbers.size() == 1 && numbers[0] > 0) {
            windowRoot_->SetMaxAppWindowNumber(static_cast<uint32_t>(numbers[0]));
        }
    }
    item = config["modeChangeHotZones"];
    if (item.IsInts()) {
        ConfigHotZones(*item.intsValue_);
    }
    item = config["splitRatios"];
    if (item.IsFloats()) {
        windowRoot_->SetSplitRatios(*item.floatsValue_);
    }
    item = config["exitSplitRatios"];
    if (item.IsFloats()) {
        windowRoot_->SetExitSplitRatios(*item.floatsValue_);
    }
    item = config["windowAnimation"];
    if (item.IsMap()) {
        ConfigWindowAnimation(item);
    }
    item = config["keyboardAnimation"];
    if (item.IsMap()) {
        ConfigKeyboardAnimation(item);
    }
    item = config["startWindowTransitionAnimation"];
    if (item.IsMap()) {
        ConfigStartingWindowAnimation(item);
    }
    item = config["windowEffect"];
    if (item.IsMap()) {
        ConfigWindowEffect(item);
    }
    item = config["floatingBottomPosY"];
    if (item.IsInts()) {
        auto numbers = *item.intsValue_;
        if (numbers.size() == 1 && numbers[0] > 0) {
            WindowLayoutPolicy::SetCascadeRectBottomPosYLimit(static_cast<uint32_t>(numbers[0]));
        }
    }
    item = config["configMainFloatingWindowAbove"].GetProp("enable");
    if (item.IsBool()) {
        WindowNodeContainer::SetConfigMainFloatingWindowAbove(item.boolValue_);
    }
    item = config["maxMainFloatingWindowNumber"];
    if (item.IsInts()) {
        auto numbers = *item.intsValue_;
        if (numbers.size() == 1 && numbers[0] > 0) {
            WindowNodeContainer::SetMaxMainFloatingWindowNumber(static_cast<uint32_t>(numbers[0]));
        }
    }
    item = config["maxFloatingWindowSize"];
    if (item.IsInts()) {
        auto numbers = *item.intsValue_;
        if (numbers.size() == 1 && numbers[0] > 0) {
            WindowLayoutPolicy::SetMaxFloatingWindowSize(static_cast<uint32_t>(numbers[0]));
        }
    }
    item = config["defaultMaximizeMode"];
    if (item.IsInts()) {
        auto numbers = *item.intsValue_;
        if (numbers.size() == 1 &&
            (numbers[0] == static_cast<int32_t>(MaximizeMode::MODE_AVOID_SYSTEM_BAR) ||
            numbers[0] == static_cast<int32_t>(MaximizeMode::MODE_FULL_FILL))) {
            maximizeMode_ = static_cast<MaximizeMode>(numbers[0]);
        }
    }
    item = config["supportTypeFloatWindow"].GetProp("enable");
    if (item.IsBool()) {
        systemConfig_.supportTypeFloatWindow_ = item.boolValue_;
    }
}

void WindowManagerService::ConfigHotZones(const std::vector<int>& numbers)
{
    if (numbers.size() == 3) { // 3 hot zones
        hotZonesConfig_.fullscreenRange_ = static_cast<uint32_t>(numbers[0]); // 0 fullscreen
        hotZonesConfig_.primaryRange_ = static_cast<uint32_t>(numbers[1]);    // 1 primary
        hotZonesConfig_.secondaryRange_ = static_cast<uint32_t>(numbers[2]);  // 2 secondary
        hotZonesConfig_.isModeChangeHotZoneConfigured_ = true;
    }
}

void WindowManagerService::ConfigDecor(const WindowManagerConfig::ConfigItem& decorConfig)
{
    WindowManagerConfig::ConfigItem item = decorConfig.GetProp("enable");
    if (item.IsBool()) {
        systemConfig_.isSystemDecorEnable_ = item.boolValue_;
        std::vector<std::string> supportedModes;
        item = decorConfig["supportedMode"];
        if (item.IsStrings()) {
            systemConfig_.decorWindowModeSupportType_ = 0;
            supportedModes = *item.stringsValue_;
        }
        for (auto mode : supportedModes) {
            if (mode == "fullscreen") {
                systemConfig_.decorWindowModeSupportType_ |= WindowModeSupport::WINDOW_MODE_SUPPORT_FULLSCREEN;
            } else if (mode == "floating") {
                systemConfig_.decorWindowModeSupportType_ |= WindowModeSupport::WINDOW_MODE_SUPPORT_FLOATING;
            } else if (mode == "pip") {
                systemConfig_.decorWindowModeSupportType_ |= WindowModeSupport::WINDOW_MODE_SUPPORT_PIP;
            } else if (mode == "split") {
                systemConfig_.decorWindowModeSupportType_ |= WindowModeSupport::WINDOW_MODE_SUPPORT_SPLIT_PRIMARY |
                    WindowModeSupport::WINDOW_MODE_SUPPORT_SPLIT_SECONDARY;
            } else {
                WLOGFW("Invalid supporedMode");
                systemConfig_.decorWindowModeSupportType_ = WindowModeSupport::WINDOW_MODE_SUPPORT_ALL;
                break;
            }
        }
    }
}

void WindowManagerService::ConfigWindowAnimation(const WindowManagerConfig::ConfigItem& animeConfig)
{
    auto& windowAnimationConfig = WindowNodeContainer::GetAnimationConfigRef().windowAnimationConfig_;
    WindowManagerConfig::ConfigItem item = animeConfig["timing"];
    if (item.IsMap() && item.mapValue_->count("curve")) {
        windowAnimationConfig.animationTiming_.timingCurve_ = CreateCurve(item["curve"]);
    }
    item = animeConfig["timing"]["duration"];
    if (item.IsInts()) {
        auto numbers = *item.intsValue_;
        if (numbers.size() == 1) { // duration
            windowAnimationConfig.animationTiming_.timingProtocol_ =
                RSAnimationTimingProtocol(numbers[0]);
        }
    }
    item = animeConfig["scale"];
    if (item.IsFloats()) {
        auto numbers = *item.floatsValue_;
        if (numbers.size() == 1) { // 1 xy scale
            windowAnimationConfig.scale_.x_ =
            windowAnimationConfig.scale_.y_ = numbers[0]; // 0 xy scale
        } else if (numbers.size() == 2) { // 2 x,y sclae
            windowAnimationConfig.scale_.x_ = numbers[0]; // 0 x scale
            windowAnimationConfig.scale_.y_ = numbers[1]; // 1 y scale
        } else if (numbers.size() == 3) { // 3 x,y,z scale
            windowAnimationConfig.scale_ = Vector3f(&numbers[0]);
        }
    }
    item = animeConfig["rotation"];
    if (item.IsFloats() && item.floatsValue_->size() == 4) { // 4 (axix,angle)
        windowAnimationConfig.rotation_ = Vector4f(item.floatsValue_->data());
    }
    item = animeConfig["translate"];
    if (item.IsFloats()) {
        auto numbers = *item.floatsValue_;
        if (numbers.size() == 2) { // 2 translate xy
            windowAnimationConfig.translate_.x_ = numbers[0]; // 0 translate x
            windowAnimationConfig.translate_.y_ = numbers[1]; // 1 translate y
        } else if (numbers.size() == 3) { // 3 translate xyz
            windowAnimationConfig.translate_.x_ = numbers[0]; // 0 translate x
            windowAnimationConfig.translate_.y_ = numbers[1]; // 1 translate y
            windowAnimationConfig.translate_.z_ = numbers[2]; // 2 translate z
        }
    }
    item = animeConfig["opacity"];
    if (item.IsFloats()) {
        auto numbers = *item.floatsValue_;
        numbers.size() == 1 ? (windowAnimationConfig.opacity_ = numbers[0]) : float();
    }
}

void WindowManagerService::ConfigKeyboardAnimation(const WindowManagerConfig::ConfigItem& animeConfig)
{
    auto& animationConfig = WindowNodeContainer::GetAnimationConfigRef();
    WindowManagerConfig::ConfigItem inItem = animeConfig["animationIn"]["timing"];
    if (inItem.IsMap() && inItem.mapValue_ != nullptr && inItem.mapValue_->count("curve")) {
        CreateKeyboardCurve(inItem, animationConfig.keyboardAnimationIn_, systemConfig_.animationIn_);
    }

    WindowManagerConfig::ConfigItem outItem = animeConfig["animationOut"]["timing"];
    if (outItem.IsMap() && outItem.mapValue_ != nullptr && outItem.mapValue_->count("curve")) {
        CreateKeyboardCurve(outItem, animationConfig.keyboardAnimationOut_, systemConfig_.animationOut_);
    }
    WLOGFI("curveIn:[%{public}s, %{public}u], curveOut:[%{public}s, %{public}u]",
        systemConfig_.animationIn_.curveType_.c_str(), systemConfig_.animationIn_.duration_,
        systemConfig_.animationOut_.curveType_.c_str(), systemConfig_.animationOut_.duration_);
}

void WindowManagerService::ConfigStartingWindowAnimation(const WindowManagerConfig::ConfigItem& animeConfig)
{
    WindowManagerConfig::ConfigItem item = animeConfig.GetProp("enable");
    if (item.IsBool()) {
        StartingWindow::transAnimateEnable_ = item.boolValue_;
    }
    auto& startWinAnimationConfig = WindowNodeContainer::GetAnimationConfigRef().startWinAnimationConfig_;
    item = animeConfig["timing"];
    if (item.IsMap() && item.mapValue_->count("curve")) {
        startWinAnimationConfig.timingCurve_ = CreateCurve(item["curve"]);
    }
    item = animeConfig["timing"]["duration"];
    if (item.IsInts()) {
        auto numbers = *item.intsValue_;
        if (numbers.size() == 1) { // duration
            startWinAnimationConfig.timingProtocol_ = RSAnimationTimingProtocol(numbers[0]);
        }
    }
    item = animeConfig["opacityStart"];
    if (item.IsFloats()) {
        auto numbers = *item.floatsValue_;
        numbers.size() == 1 ? (startWinAnimationConfig.opacityStart_ = numbers[0]) : float();
    }
    item = animeConfig["opacityEnd"];
    if (item.IsFloats()) {
        auto numbers = *item.floatsValue_;
        numbers.size() == 1 ? (startWinAnimationConfig.opacityEnd_ = numbers[0]) : float();
    }
}

bool WindowManagerService::ConfigAppWindowCornerRadius(const WindowManagerConfig::ConfigItem& item, float& out)
{
    std::map<std::string, float> stringToCornerRadius = {
        {"off", 0.0f}, {"defaultCornerRadiusXS", 4.0f}, {"defaultCornerRadiusS", 8.0f},
        {"defaultCornerRadiusM", 12.0f}, {"defaultCornerRadiusL", 16.0f}, {"defaultCornerRadiusXL", 24.0f}
    };

    if (item.IsString()) {
        auto value = item.stringValue_;
        if (stringToCornerRadius.find(value) != stringToCornerRadius.end()) {
            out = stringToCornerRadius[value];
            return true;
        }
    }
    return false;
}

bool WindowManagerService::ConfigAppWindowShadow(const WindowManagerConfig::ConfigItem& shadowConfig,
    WindowShadowParameters& outShadow)
{
    WindowManagerConfig::ConfigItem item = shadowConfig["elevation"];
    if (item.IsFloats()) {
        auto elevation = *item.floatsValue_;
        if (elevation.size() != 1 || MathHelper::LessNotEqual(elevation[0], 0.0)) {
            return false;
        }
        outShadow.elevation_ = elevation[0];
    }

    item = shadowConfig["color"];
    if (item.IsString()) {
        auto color = item.stringValue_;
        uint32_t colorValue;
        if (!ColorParser::Parse(color, colorValue)) {
            return false;
        }
        outShadow.color_ = color;
    }

    item = shadowConfig["offsetX"];
    if (item.IsFloats()) {
        auto offsetX = *item.floatsValue_;
        if (offsetX.size() != 1) {
            return false;
        }
        outShadow.offsetX_ = offsetX[0];
    }

    item = shadowConfig["offsetY"];
    if (item.IsFloats()) {
        auto offsetY = *item.floatsValue_;
        if (offsetY.size() != 1) {
            return false;
        }
        outShadow.offsetY_ = offsetY[0];
    }

    item = shadowConfig["alpha"];
    if (item.IsFloats()) {
        auto alpha = *item.floatsValue_;
        if (alpha.size() != 1 ||
            (MathHelper::LessNotEqual(alpha[0], 0.0) && MathHelper::GreatNotEqual(alpha[0], 1.0))) {
            return false;
        }
        outShadow.alpha_ = alpha[0];
    }

    item = shadowConfig["radius"];
    if (item.IsFloats()) {
        auto radius = *item.floatsValue_;
        if (radius.size() != 1 || MathHelper::LessNotEqual(radius[0], 0.0)) {
            return false;
        }
        outShadow.radius_ = radius[0];
    }

    return true;
}

void WindowManagerService::ConfigWindowEffect(const WindowManagerConfig::ConfigItem& effectConfig)
{
    AppWindowEffectConfig config;
    AppWindowEffectConfig systemEffectConfig;
    // config corner radius
    WindowManagerConfig::ConfigItem item = effectConfig["appWindows"]["cornerRadius"];
    if (item.IsMap()) {
        if (ConfigAppWindowCornerRadius(item["fullScreen"], config.fullScreenCornerRadius_) &&
            ConfigAppWindowCornerRadius(item["split"], config.splitCornerRadius_) &&
            ConfigAppWindowCornerRadius(item["float"], config.floatCornerRadius_)) {
            systemEffectConfig = config;
        }
    }

    // config shadow
    item = effectConfig["appWindows"]["shadow"]["focused"];
    if (item.IsMap()) {
        if (ConfigAppWindowShadow(item, config.focusedShadow_)) {
            systemEffectConfig.focusedShadow_ = config.focusedShadow_;
        }
    }

    item = effectConfig["appWindows"]["shadow"]["unfocused"];
    if (item.IsMap()) {
        if (ConfigAppWindowShadow(item, config.unfocusedShadow_)) {
            systemEffectConfig.unfocusedShadow_ = config.unfocusedShadow_;
        }
    }
    WindowSystemEffect::SetWindowSystemEffectConfig(systemEffectConfig);
}

void WindowManagerService::CreateKeyboardCurve(const WindowManagerConfig::ConfigItem& config,
    AnimationConfig::KeyboardAnimation& animateConfig, KeyboardAnimationCurve& sysCurveConfig)
{
    // parse curve params
    const WindowManagerConfig::ConfigItem& curveConfig = config["curve"];
    static std::map<std::string, RSAnimationTimingCurve> curveMap = {
        { "easeOut",           RSAnimationTimingCurve::EASE_OUT },
        { "ease",              RSAnimationTimingCurve::EASE },
        { "easeIn",            RSAnimationTimingCurve::EASE_IN },
        { "easeInOut",         RSAnimationTimingCurve::EASE_IN_OUT },
        { "default",           RSAnimationTimingCurve::DEFAULT },
        { "linear",            RSAnimationTimingCurve::LINEAR },
        { "spring",            RSAnimationTimingCurve::SPRING },
        { "interactiveSpring", RSAnimationTimingCurve::INTERACTIVE_SPRING }
    };
    RSAnimationTimingCurve curve = RSAnimationTimingCurve::EASE_OUT;
    std::string keyboardCurveName = "easeOut";
    std::vector<float> keyboardCurveParams = {};
    const auto& nameItem = curveConfig.GetProp("name");
    if (nameItem.IsString()) {
        std::string name = nameItem.stringValue_;
        if (name == "cubic" && curveConfig.IsFloats() && curveConfig.floatsValue_ != nullptr &&
            curveConfig.floatsValue_->size() == 4) { // 4: param size
            const auto& numbers = *curveConfig.floatsValue_;
            keyboardCurveName = name;
            keyboardCurveParams.assign(numbers.begin(), numbers.end());
            curve = RSAnimationTimingCurve::CreateCubicCurve(
                numbers[0],  // 0 ctrlX1
                numbers[1],  // 1 ctrlY1
                numbers[2],  // 2 ctrlX2
                numbers[3]); // 3 ctrlY2
        } else {
            auto iter = curveMap.find(name);
            if (iter != curveMap.end()) {
                keyboardCurveName = name;
                curve = iter->second;
            }
        }
    }
    animateConfig.curve_ = curve;
    sysCurveConfig.curveType_ = keyboardCurveName;
    sysCurveConfig.curveParams_.assign(keyboardCurveParams.begin(), keyboardCurveParams.end());

    // parse curve duration
    const WindowManagerConfig::ConfigItem& duration = config["duration"];
    if (duration.IsInts() && duration.intsValue_ != nullptr) {
        auto numbers = *duration.intsValue_;
        if (numbers.size() == 1) { // duration
            animateConfig.duration_ = RSAnimationTimingProtocol(numbers[0]);
            sysCurveConfig.duration_ = static_cast<uint32_t>(numbers[0]);
        }
    }
}

RSAnimationTimingCurve WindowManagerService::CreateCurve(const WindowManagerConfig::ConfigItem& curveConfig)
{
    static std::map<std::string, RSAnimationTimingCurve> curveMap = {
        { "easeOut",           RSAnimationTimingCurve::EASE_OUT },
        { "ease",              RSAnimationTimingCurve::EASE },
        { "easeIn",            RSAnimationTimingCurve::EASE_IN },
        { "easeInOut",         RSAnimationTimingCurve::EASE_IN_OUT },
        { "default",           RSAnimationTimingCurve::DEFAULT },
        { "linear",            RSAnimationTimingCurve::LINEAR },
        { "spring",            RSAnimationTimingCurve::SPRING },
        { "interactiveSpring", RSAnimationTimingCurve::INTERACTIVE_SPRING }
    };

    RSAnimationTimingCurve curve = RSAnimationTimingCurve::EASE_OUT;
    const auto& nameItem = curveConfig.GetProp("name");
    if (nameItem.IsString()) {
        std::string name = nameItem.stringValue_;
        if (name == "cubic" && curveConfig.IsFloats() && curveConfig.floatsValue_ != nullptr &&
            curveConfig.floatsValue_->size() == 4) { // 4 curve parameter
            const auto& numbers = *curveConfig.floatsValue_;
            curve = RSAnimationTimingCurve::CreateCubicCurve(
                numbers[0],  // 0 ctrlX1
                numbers[1],  // 1 ctrlY1
                numbers[2],  // 2 ctrlX2
                numbers[3]); // 3 ctrlY2
        } else {
            if (auto iter = curveMap.find(name); iter != curveMap.end()) {
                curve = iter->second;
            }
        }
    }
    return curve;
}

void WindowManagerService::OnStop()
{
    windowCommonEvent_->UnSubscriberEvent();
    WindowInnerManager::GetInstance().Stop();
    WLOGI("ready to stop service.");
}

WMError WindowManagerService::NotifyWindowTransition(
    sptr<WindowTransitionInfo>& fromInfo, sptr<WindowTransitionInfo>& toInfo, bool isFromClient)
{
    if (!isFromClient) {
        WLOGI("NotifyWindowTransition asynchronously.");
        auto task = [this, fromInfo, toInfo]() mutable {
            return windowController_->NotifyWindowTransition(fromInfo, toInfo);
        };
        PostAsyncTask(task, "NotifyWindowTransition");
        return WMError::WM_OK;
    } else {
        WLOGI("NotifyWindowTransition synchronously.");
        auto task = [this, &fromInfo, &toInfo]() {
            return windowController_->NotifyWindowTransition(fromInfo, toInfo);
        };
        return PostSyncTask(task, "NotifyWindowTransition");
    }
}

void WindowManagerService::NotifyAnimationAbilityDied(sptr<WindowTransitionInfo> info)
{
    auto task = [this, info]() mutable {
        return RemoteAnimation::NotifyAnimationAbilityDied(info);
    };
    PostAsyncTask(task, "NotifyAnimationAbilityDied");
}

WMError WindowManagerService::GetFocusWindowInfo(sptr<IRemoteObject>& abilityToken, DisplayId displayId)
{
    auto task = [this, &abilityToken]() {
        return windowController_->GetFocusWindowInfo(abilityToken);
    };
    return PostSyncTask(task, "GetFocusWindowInfo");
}

void WindowManagerService::StartingWindow(sptr<WindowTransitionInfo> info, std::shared_ptr<Media::PixelMap> pixelMap,
    bool isColdStart, uint32_t bkgColor)
{
    if (!startingOpen_) {
        TLOGI(WmsLogTag::WMS_STARTUP_PAGE, "startingWindow not open!");
        return;
    }
    if (info) {
        info->isSystemCalling_ = Permission::IsSystemCalling();
    }
    auto task = [this, info, pixelMap, isColdStart, bkgColor]() {
        windowController_->StartingWindow(info, pixelMap, bkgColor, isColdStart);
    };
    PostAsyncTask(task, "StartingWindow");
}

void WindowManagerService::CancelStartingWindow(sptr<IRemoteObject> abilityToken)
{
    TLOGI(WmsLogTag::WMS_STARTUP_PAGE, "begin");
    if (!startingOpen_) {
        TLOGI(WmsLogTag::WMS_STARTUP_PAGE, "startingWindow not open!");
        return;
    }
    auto task = [this, abilityToken]() {
        windowController_->CancelStartingWindow(abilityToken);
    };
    PostAsyncTask(task, "CancelStartingWindow");
}

WMError WindowManagerService::MoveMissionsToForeground(const std::vector<int32_t>& missionIds, int32_t topMissionId)
{
    if (windowGroupMgr_) {
        auto task = [this, &missionIds, topMissionId]() {
            WMError res = windowGroupMgr_->MoveMissionsToForeground(missionIds, topMissionId);
            // no need to return inner error to caller
            if (res > WMError::WM_ERROR_NEED_REPORT_BASE) {
                return res;
            }
            return WMError::WM_OK;
        };
        return PostSyncTask(task, "MoveMissionsToForeground");
    }
    return WMError::WM_ERROR_NULLPTR;
}

WMError WindowManagerService::MoveMissionsToBackground(const std::vector<int32_t>& missionIds,
    std::vector<int32_t>& result)
{
    if (windowGroupMgr_) {
        auto task = [this, &missionIds, &result]() {
            WMError res = windowGroupMgr_->MoveMissionsToBackground(missionIds, result);
            // no need to return wms inner error to caller
            if (res > WMError::WM_ERROR_NEED_REPORT_BASE) {
                return res;
            }
            return WMError::WM_OK;
        };
        return PostSyncTask(task, "MoveMissionsToBackground");
    }
    return WMError::WM_ERROR_NULLPTR;
}


bool WindowManagerService::CheckAnimationPermission(const sptr<WindowProperty>& property) const
{
    WindowType type = property->GetWindowType();
    // If the animation type is NONE or the window type is WINDOW_TYPE_INPUT_METHOD_FLOAT
    if (property->GetAnimationFlag() == static_cast<uint32_t>(WindowAnimation::NONE) ||
        type == WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT) {
        return true;
    }
    // If the animation type is DEFAULT and the window type is AppWindow
    if (property->GetAnimationFlag() == static_cast<uint32_t>(WindowAnimation::DEFAULT) &&
        WindowHelper::IsAppWindow(type)) {
        return true;
    }
    // If the animation type is CUSTOM
    if (Permission::IsSystemCalling() || Permission::IsStartByHdcd()) {
        WLOGFD("check IsSystemCalling permission success, show with animation calling.");
        return true;
    }
    WLOGFE("check animation permission failed");
    return false;
}

bool WindowManagerService::CheckSystemWindowPermission(const sptr<WindowProperty>& property) const
{
    WindowType type = property->GetWindowType();
    if (!WindowHelper::IsSystemWindow(type)) {
        // type is not system
        return true;
    }
    if ((type == WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT || type == WindowType::WINDOW_TYPE_INPUT_METHOD_STATUS_BAR)
        && Permission::IsStartByInputMethod()) {
        // WINDOW_TYPE_INPUT_METHOD_FLOAT counld be created by input method app
        WLOGFD("check create permission success, input method app create input method window.");
        return true;
    }
    if (type == WindowType::WINDOW_TYPE_DRAGGING_EFFECT || type == WindowType::WINDOW_TYPE_SYSTEM_ALARM_WINDOW ||
        type == WindowType::WINDOW_TYPE_TOAST || type == WindowType::WINDOW_TYPE_DIALOG) {
        // some system types counld be created by normal app
        return true;
    }
    if (type == WindowType::WINDOW_TYPE_FLOAT &&
        Permission::CheckCallingPermission("ohos.permission.SYSTEM_FLOAT_WINDOW")) {
        // WINDOW_TYPE_FLOAT counld be created by normal app with the corresponding permission
        WLOGFD("check create permission success, normal app create float window with request permission.");
        return true;
    }
    if (Permission::IsSystemCalling() || Permission::IsStartByHdcd()) {
        WLOGFD("check create permission success, create with system calling.");
        return true;
    }
    WLOGFE("check system window permission failed.");
    return false;
}

WMError WindowManagerService::CreateWindow(sptr<IWindow>& window, sptr<WindowProperty>& property,
    const std::shared_ptr<RSSurfaceNode>& surfaceNode, uint32_t& windowId, sptr<IRemoteObject> token)
{
    if (!window || property == nullptr || surfaceNode == nullptr || !window->AsObject()) {
        WLOGFE("window is invalid");
        return WMError::WM_ERROR_NULLPTR;
    }
    if (!CheckSystemWindowPermission(property)) {
        WLOGFE("create system window permission denied!");
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    int pid = IPCSkeleton::GetCallingRealPid();
    int uid = IPCSkeleton::GetCallingUid();
    property->isSystemCalling_ = Permission::IsSystemCalling();
    if (rsUiDirector_) {
        RSAdapterUtil::SetRSUIContext(surfaceNode, rsUiDirector_->GetRSUIContext(), true);
    }
    auto task = [this, pid, uid, &window, &property, &surfaceNode, &windowId, &token]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "wms:CreateWindow(%u)", windowId);
        return windowController_->CreateWindow(window, property, surfaceNode, windowId, token, pid, uid);
    };
    WMError ret = PostSyncTask(task, "CreateWindow");
    accessTokenIdMaps_.insert(std::pair(windowId, IPCSkeleton::GetCallingTokenID()));
    return ret;
}

WMError WindowManagerService::AddWindow(sptr<WindowProperty>& property)
{
    if (property == nullptr) {
        WLOGFE("property is nullptr");
        return WMError::WM_ERROR_NULLPTR;
    }
    if (!CheckSystemWindowPermission(property) || !CheckAnimationPermission(property)) {
        WLOGFE("add window permission denied!");
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    auto task = [this, &property]() {
        windowShowPerformReport_->start();
        Rect rect = property->GetRequestRect();
        uint32_t windowId = property->GetWindowId();
        WLOGI("[WMS] Add: %{public}5d %{public}4d %{public}4d %{public}4d [%{public}4d %{public}4d " \
            "%{public}4d %{public}4d]", windowId, property->GetWindowType(), property->GetWindowMode(),
            property->GetWindowFlags(), rect.posX_, rect.posY_, rect.width_, rect.height_);
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "wms:AddWindow(%u)", windowId);
        WMError res = windowController_->AddWindowNode(property);
        if (property->GetWindowType() == WindowType::WINDOW_TYPE_DRAGGING_EFFECT) {
            dragController_->StartDrag(windowId);
        }
        if (res == WMError::WM_OK) {
            windowShowPerformReport_->end();
        }
        return res;
    };
    return PostSyncTask(task, "AddWindow");
}

WMError WindowManagerService::RemoveWindow(uint32_t windowId, bool isFromInnerkits)
{
    if (!isFromInnerkits && !Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        WLOGFE("remove window permission denied!");
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    if (!accessTokenIdMaps_.isExist(windowId, IPCSkeleton::GetCallingTokenID())) {
        WLOGI("Operation rejected");
        return WMError::WM_ERROR_INVALID_OPERATION;
    }
    auto task = [this, windowId]() {
        WLOGI("[WMS] Remove: %{public}u", windowId);
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "wms:RemoveWindow(%u)", windowId);
        WindowInnerManager::GetInstance().NotifyWindowRemovedOrDestroyed(windowId);
        WMError res = windowController_->RecoverInputEventToClient(windowId);
        if (res != WMError::WM_OK) {
            return res;
        }
        return windowController_->RemoveWindowNode(windowId);
    };
    return PostSyncTask(task, "RemoveWindow");
}

WMError WindowManagerService::DestroyWindow(uint32_t windowId, bool onlySelf)
{
    if (!accessTokenIdMaps_.isExistAndRemove(windowId, IPCSkeleton::GetCallingTokenID())) {
        WLOGI("Operation rejected");
        return WMError::WM_ERROR_INVALID_OPERATION;
    }
    auto task = [this, windowId, onlySelf]() {
        auto node = windowRoot_->GetWindowNode(windowId);
        if (node == nullptr) {
            return WMError::WM_ERROR_NULLPTR;
        }
        node->stateMachine_.SetDestroyTaskParam(onlySelf);
        auto func = [this, windowId]() {
            WLOGI("[WMS] Destroy: %{public}u", windowId);
            HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "wms:DestroyWindow(%u)", windowId);
            WindowInnerManager::GetInstance().NotifyWindowRemovedOrDestroyed(windowId);
            windowGroupMgr_->OnWindowDestroyed(windowId);
            auto node = windowRoot_->GetWindowNode(windowId);
            if (node == nullptr) {
                return WMError::WM_OK;
            }
            if (node->GetWindowType() == WindowType::WINDOW_TYPE_DRAGGING_EFFECT) {
                dragController_->FinishDrag(windowId);
            }
            return windowController_->DestroyWindow(windowId, node->stateMachine_.GetDestroyTaskParam());
        };
        if (RemoteAnimation::IsRemoteAnimationEnabledAndFirst(node->GetDisplayId()) &&
            node->stateMachine_.IsRemoteAnimationPlaying()) {
            WLOGI("SetDestroyTask id:%{public}u", node->GetWindowId());
            node->stateMachine_.SetDestroyTask(func);
            return WMError::WM_OK;
        }
        WLOGI("DestroyWindow windowId: %{public}u, name:%{public}s state: %{public}u",
            node->GetWindowId(), node->GetWindowName().c_str(),
            static_cast<uint32_t>(node->stateMachine_.GetCurrentState()));
        return func();
    };
    return PostSyncTask(task, "DestroyWindow");
}

WMError WindowManagerService::RequestFocus(uint32_t windowId)
{
    auto task = [this, windowId]() {
        WLOGI("[WMS] RequestFocus: %{public}u", windowId);
        return windowController_->RequestFocus(windowId);
    };
    return PostSyncTask(task, "RequestFocus");
}

AvoidArea WindowManagerService::GetAvoidAreaByType(uint32_t windowId, AvoidAreaType avoidAreaType, const Rect& rect)
{
    auto task = [this, windowId, avoidAreaType]() {
        WLOGI("[WMS] GetAvoidAreaByType: %{public}u, Type: %{public}u", windowId,
            static_cast<uint32_t>(avoidAreaType));
        return windowController_->GetAvoidAreaByType(windowId, avoidAreaType);
    };
    return PostSyncTask(task, "GetAvoidAreaByType");
}

WMError WindowManagerService::RegisterWindowManagerAgent(WindowManagerAgentType type,
    const sptr<IWindowManagerAgent>& windowManagerAgent)
{
    if (!Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        WLOGFE("register windowManager agent permission denied!");
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    if ((windowManagerAgent == nullptr) || (windowManagerAgent->AsObject() == nullptr)) {
        WLOGFE("windowManagerAgent is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    auto task = [this, &windowManagerAgent, type]() {
        WMError ret = WindowManagerAgentController::GetInstance().RegisterWindowManagerAgent(windowManagerAgent, type);
        if (type == WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_SYSTEM_BAR) { // if system bar, notify once
            windowController_->NotifySystemBarTints();
        }
        return ret;
    };
    return PostSyncTask(task, "RegisterWindowManagerAgent");
}

WMError WindowManagerService::UnregisterWindowManagerAgent(WindowManagerAgentType type,
    const sptr<IWindowManagerAgent>& windowManagerAgent)
{
    if (!Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        WLOGFE("unregister windowManager agent permission denied!");
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    if ((windowManagerAgent == nullptr) || (windowManagerAgent->AsObject() == nullptr)) {
        WLOGFE("windowManagerAgent is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    auto task = [this, &windowManagerAgent, type]() {
        return WindowManagerAgentController::GetInstance().UnregisterWindowManagerAgent(windowManagerAgent, type);
    };
    return PostSyncTask(task, "UnregisterWindowManagerAgent");
}

WMError WindowManagerService::SetWindowAnimationController(const sptr<RSIWindowAnimationController>& controller)
{
    if (!Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        WLOGFE("set window animation controller permission denied!");
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    if (controller == nullptr) {
        WLOGFE("RSWindowAnimation: Failed to set window animation controller, controller is null!");
        return WMError::WM_ERROR_NULLPTR;
    }

    sptr<AgentDeathRecipient> deathRecipient = new AgentDeathRecipient(
        [this](sptr<IRemoteObject>& remoteObject) {
            auto task = [&remoteObject]() {
                RemoteAnimation::OnRemoteDie(remoteObject);
            };
            PostVoidSyncTask(task, "OnRemoteDie");
        }
    );
    controller->AsObject()->AddDeathRecipient(deathRecipient);
    RemoteAnimation::SetWindowControllerAndRoot(windowController_, windowRoot_);
    RemoteAnimation::SetMainTaskHandler(handler_);
    auto task = [this, &controller]() {
        WMError ret = windowController_->SetWindowAnimationController(controller);
        RemoteAnimation::SetAnimationFirst(system::GetParameter("persist.window.af.enabled", "1") == "1");
        return ret;
    };
    return PostSyncTask(task, "SetWindowAnimationController");
}

void WindowManagerService::OnWindowEvent(Event event, const sptr<IRemoteObject>& remoteObject)
{
    if (event == Event::REMOTE_DIED) {
        auto task = [this, &remoteObject]() {
            uint32_t windowId = windowRoot_->GetWindowIdByObject(remoteObject);
            auto node = windowRoot_->GetWindowNode(windowId);
            if (node == nullptr) {
                WLOGFD("window node is nullptr, REMOTE_DIED no need to destroy");
                return;
            }
            WLOGI("window %{public}u received REMOTE_DIED", windowId);
            node->stateMachine_.SetDestroyTaskParam(true);
            auto func = [this, windowId]() {
                auto node = windowRoot_->GetWindowNode(windowId);
                if (node == nullptr) {
                    WLOGFD("window node is nullptr");
                    return;
                }
                if (node->GetWindowType() == WindowType::WINDOW_TYPE_DRAGGING_EFFECT) {
                    dragController_->FinishDrag(windowId);
                }
                WindowInnerManager::GetInstance().NotifyWindowRemovedOrDestroyed(windowId);
                windowGroupMgr_->OnWindowDestroyed(windowId);
                windowController_->DestroyWindow(windowId, node->stateMachine_.GetDestroyTaskParam());
            };

            if (node->GetWindowType() == WindowType::WINDOW_TYPE_DESKTOP) {
                RemoteAnimation::OnRemoteDie(remoteObject);
            }
            if (RemoteAnimation::IsRemoteAnimationEnabledAndFirst(node->GetDisplayId()) &&
                node->stateMachine_.IsRemoteAnimationPlaying()) {
                WLOGI("set destroy task windowId:%{public}u", node->GetWindowId());
                node->stateMachine_.SetDestroyTask(func);
                handler_->PostTask(func, "destroyTimeOutTask", 6000); // 6000 is time out 6s
                return;
            }
            func();
        };
        PostVoidSyncTask(task, "OnWindowEvent");
    }
}

void WindowManagerService::NotifyDisplayStateChange(DisplayId defaultDisplayId, sptr<DisplayInfo> displayInfo,
    const std::map<DisplayId, sptr<DisplayInfo>>& displayInfoMap, DisplayStateChangeType type)
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "wms:NotifyDisplayStateChange(%u)", type);
    DisplayId displayId = (displayInfo == nullptr) ? DISPLAY_ID_INVALID : displayInfo->GetDisplayId();
    if (type == DisplayStateChangeType::FREEZE) {
        freezeDisplayController_->FreezeDisplay(displayId);
    } else if (type == DisplayStateChangeType::UNFREEZE) {
        freezeDisplayController_->UnfreezeDisplay(displayId);
        /*
         * Set 'InnerInputManager Listener' to MMI, ensure that the listener
         * for move/drag won't be replaced by freeze-display-window
         */
        WindowInnerManager::GetInstance().SetInputEventConsumer();
    } else {
        auto task = [this, defaultDisplayId, displayInfo, displayInfoMap, type]() mutable {
            windowController_->NotifyDisplayStateChange(defaultDisplayId, displayInfo, displayInfoMap, type);
            windowGroupMgr_->OnDisplayStateChange(defaultDisplayId, displayInfo, displayInfoMap, type);
        };
        PostAsyncTask(task, "NotifyDisplayStateChange");
    }
}

void DisplayChangeListener::OnDisplayStateChange(DisplayId defaultDisplayId, sptr<DisplayInfo> displayInfo,
    const std::map<DisplayId, sptr<DisplayInfo>>& displayInfoMap, DisplayStateChangeType type)
{
    WindowManagerService::GetInstance().NotifyDisplayStateChange(defaultDisplayId, displayInfo, displayInfoMap, type);
}

void DisplayChangeListener::OnScreenshot(DisplayId displayId)
{
    WindowManagerService::GetInstance().OnScreenshot(displayId);
}

void WindowManagerService::NotifyServerReadyToMoveOrDrag(uint32_t windowId, sptr<WindowProperty>& windowProperty,
    sptr<MoveDragProperty>& moveDragProperty)
{
    if (windowProperty == nullptr || moveDragProperty == nullptr) {
        WLOGFE("windowProperty or moveDragProperty is invalid");
        return;
    }

    auto task = [this, windowId, windowProperty, moveDragProperty]() mutable {
        if (moveDragProperty->startDragFlag_ || moveDragProperty->startMoveFlag_) {
            bool res = WindowInnerManager::GetInstance().NotifyServerReadyToMoveOrDrag(windowId,
                windowProperty, moveDragProperty);
            if (!res) {
                WLOGFE("invalid operation");
                return;
            }
            windowController_->InterceptInputEventToServer(windowId);
        }
        windowController_->NotifyServerReadyToMoveOrDrag(windowId, moveDragProperty);
    };
    PostAsyncTask(task, "NotifyServerReadyToMoveOrDrag");
}

void WindowManagerService::ProcessPointDown(uint32_t windowId, bool isPointDown)
{
    auto task = [this, windowId, isPointDown]() {
        windowController_->ProcessPointDown(windowId, isPointDown);
    };
    PostAsyncTask(task, "ProcessPointDown");
}

void WindowManagerService::ProcessPointUp(uint32_t windowId)
{
    auto task = [this, windowId]() {
        WindowInnerManager::GetInstance().NotifyWindowEndUpMovingOrDragging(windowId);
        windowController_->RecoverInputEventToClient(windowId);
        windowController_->ProcessPointUp(windowId);
    };
    PostAsyncTask(task, "ProcessPointUp");
}

void WindowManagerService::NotifyWindowClientPointUp(uint32_t windowId,
    const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    auto task = [this, windowId, pointerEvent]() mutable {
        windowController_->NotifyWindowClientPointUp(windowId, pointerEvent);
    };
    PostAsyncTask(task, "NotifyWindowClientPointUp");
}

WMError WindowManagerService::MinimizeAllAppWindows(DisplayId displayId, int32_t excludeWindowId)
{
    if (!Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        WLOGFE("minimize all appWindows permission denied!");
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    auto task = [this, displayId, excludeWindowId]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "wms:MinimizeAllAppWindows(%" PRIu64")", displayId);
        WLOGI("displayId %{public}" PRIu64", excludeWindowId : %{public}d", displayId, excludeWindowId);
        windowController_->MinimizeAllAppWindows(displayId, excludeWindowId);
    };
    PostAsyncTask(task, "MinimizeAllAppWindows");
    return WMError::WM_OK;
}

WMError WindowManagerService::ToggleShownStateForAllAppWindows()
{
    if (!Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        WLOGFE("toggle shown state for all appwindows permission denied!");
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    auto task = [this]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "wms:ToggleShownStateForAllAppWindows");
        return windowController_->ToggleShownStateForAllAppWindows();
    };
    PostAsyncTask(task, "ToggleShownStateForAllAppWindows");
    return WMError::WM_OK;
}

WMError WindowManagerService::GetTopWindowId(uint32_t mainWinId, uint32_t& topWinId)
{
    auto task = [this, &topWinId, mainWinId]() {
        return windowController_->GetTopWindowId(mainWinId, topWinId);
    };
    return PostSyncTask(task, "GetTopWindowId");
}

WMError WindowManagerService::SetWindowLayoutMode(WindowLayoutMode mode)
{
    if (!Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        WLOGFE("set window layout mode permission denied!");
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    auto task = [this, mode]() {
        WLOGI("layoutMode: %{public}u", mode);
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "wms:SetWindowLayoutMode");
        return windowController_->SetWindowLayoutMode(mode);
    };
    return PostSyncTask(task, "SetWindowLayoutMode");
}

WMError WindowManagerService::NotifyScreenshotEvent(ScreenshotEventType type)
{
    if (!Permission::IsSystemCalling()) {
        TLOGI(WmsLogTag::WMS_ATTRIBUTE, "notify screen shot event type permission denied!");
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "event: %{public}d", type);
    auto task = [this, type]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "wms:NotifyScreenshotEvent");
        return windowController_->NotifyScreenshotEvent(type);
    };
    return PostSyncTask(task, "SetWindowLayoutMode");
}

WMError WindowManagerService::UpdateProperty(sptr<WindowProperty>& windowProperty, PropertyChangeAction action,
    bool isAsyncTask)
{
    if (windowProperty == nullptr) {
        WLOGFE("windowProperty is nullptr");
        return WMError::WM_ERROR_NULLPTR;
    }

    if ((windowProperty->GetWindowFlags() == static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_FORBID_SPLIT_MOVE) ||
        action == PropertyChangeAction::ACTION_UPDATE_TRANSFORM_PROPERTY) &&
        !Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        WLOGFE("SetForbidSplitMove or SetShowWhenLocked or SetTranform or SetTurnScreenOn permission denied!");
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }

    WindowType type = windowProperty->GetWindowType();
    if (type == WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT ||
        type == WindowType::WINDOW_TYPE_INPUT_METHOD_STATUS_BAR) {
        if (!Permission::IsStartByInputMethod()) {
            WLOGI("Keyboard only hide by input method it'self, operation rejected.");
            return WMError::WM_ERROR_INVALID_OPERATION;
        }
    } else if (!accessTokenIdMaps_.isExist(windowProperty->GetWindowId(), IPCSkeleton::GetCallingTokenID()) &&
        !Permission::IsSystemCalling()) {
        WLOGI("Operation rejected");
        return WMError::WM_ERROR_INVALID_OPERATION;
    }

    if (action == PropertyChangeAction::ACTION_UPDATE_PRIVACY_MODE &&
        !Permission::CheckCallingPermission("ohos.permission.PRIVACY_WINDOW")) {
        WLOGFE("Set privacy mode permission denied!");
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }

    windowProperty->isSystemCalling_ = Permission::IsSystemCalling();
    if (action == PropertyChangeAction::ACTION_UPDATE_TRANSFORM_PROPERTY) {
        auto task = [this, windowProperty, action]() mutable {
            windowController_->UpdateProperty(windowProperty, action);
            return WMError::WM_OK;
        };
        return PostSyncTask(task, "UpdateProperty");
    }

    if (isAsyncTask || action == PropertyChangeAction::ACTION_UPDATE_RECT) {
        auto task = [this, windowProperty, action]() mutable {
            HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "wms:UpdateProperty");
            WMError res = windowController_->UpdateProperty(windowProperty, action);
            if (action == PropertyChangeAction::ACTION_UPDATE_RECT && res == WMError::WM_OK &&
                (IsMoveToOrDragMove(windowProperty->GetWindowSizeChangeReason()))) {
                dragController_->UpdateDragInfo(windowProperty->GetWindowId());
            }
        };
        PostAsyncTask(task, "UpdateProperty");
        return WMError::WM_OK;
    }

    auto task = [this, &windowProperty, action]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "wms:UpdateProperty");
        WMError res = windowController_->UpdateProperty(windowProperty, action);
        if (action == PropertyChangeAction::ACTION_UPDATE_RECT && res == WMError::WM_OK &&
            IsMoveToOrDragMove(windowProperty->GetWindowSizeChangeReason())) {
            dragController_->UpdateDragInfo(windowProperty->GetWindowId());
        }
        return res;
    };
    return PostSyncTask(task, "UpdateProperty");
}

WMError WindowManagerService::SetWindowGravity(uint32_t windowId, WindowGravity gravity, uint32_t percent)
{
    auto task = [this, windowId, gravity, percent]() {
        WMError res = windowController_->SetWindowGravity(windowId, gravity, percent);
        return res;
    };
    return PostSyncTask(task, "SetWindowGravity");
}

WMError WindowManagerService::GetAccessibilityWindowInfo(std::vector<sptr<AccessibilityWindowInfo>>& infos)
{
    if (!Permission::IsSystemServiceCalling()) {
        WLOGFE("get accessibility window info permission denied!");
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    auto task = [this, &infos]() {
        return windowController_->GetAccessibilityWindowInfo(infos);
    };
    return PostSyncTask(task, "GetAccessibilityWindowInfo");
}

WMError WindowManagerService::GetUnreliableWindowInfo(int32_t windowId,
    std::vector<sptr<UnreliableWindowInfo>>& infos)
{
    if (!Permission::IsSystemServiceCalling()) {
        WLOGFE("get unreliable window info permission denied!");
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    auto task = [this, windowId, &infos]() {
        return windowController_->GetUnreliableWindowInfo(windowId, infos);
    };
    return PostSyncTask(task, "GetUnreliableWindowInfo");
}

WMError WindowManagerService::GetVisibilityWindowInfo(std::vector<sptr<WindowVisibilityInfo>>& infos)
{
    auto task = [this, &infos]() {
        return windowController_->GetVisibilityWindowInfo(infos);
    };
    return PostSyncTask(task, "GetVisibilityWindowInfo");
}

/** @note @window.hierarchy */
WMError WindowManagerService::RaiseToAppTop(uint32_t windowId)
{
    auto task = [this, windowId]() {
        return windowController_->RaiseToAppTop(windowId);
    };
    return PostSyncTask(task, "RaiseToAppTop");
}

std::shared_ptr<Media::PixelMap> WindowManagerService::GetSnapshot(int32_t windowId)
{
    if (!Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        WLOGFE("permission denied!");
        return nullptr;
    }
    auto task = [this, windowId]() {
        return windowController_->GetSnapshot(windowId);
    };
    return PostSyncTask(task, "GetSnapshot");
}

void WindowManagerService::DispatchKeyEvent(uint32_t windowId, std::shared_ptr<MMI::KeyEvent> event)
{
    auto task = [this, windowId, event]() {
        windowController_->DispatchKeyEvent(windowId, event);
    };
    PostVoidSyncTask(task, "DispatchKeyEvent");
}

void WindowManagerService::NotifyDumpInfoResult(const std::vector<std::string>& info)
{
    if (windowDumper_) {
        windowDumper_->dumpInfoFuture_.SetValue(info);
    }
}

WMError WindowManagerService::GetWindowAnimationTargets(std::vector<uint32_t> missionIds,
    std::vector<sptr<RSWindowAnimationTarget>>& targets)
{
    if (!Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        WLOGFE("get window animation targets permission denied!");
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    auto task = [this, missionIds, &targets]() {
        return RemoteAnimation::GetWindowAnimationTargets(missionIds, targets);
    };
    return PostSyncTask(task, "GetWindowAnimationTargets");
}

WMError WindowManagerService::GetSystemConfig(SystemConfig& systemConfig)
{
    systemConfig = systemConfig_;
    return WMError::WM_OK;
}

WMError WindowManagerService::GetModeChangeHotZones(DisplayId displayId, ModeChangeHotZones& hotZones)
{
    if (!hotZonesConfig_.isModeChangeHotZoneConfigured_) {
        return WMError::WM_DO_NOTHING;
    }

    return windowController_->GetModeChangeHotZones(displayId, hotZones, hotZonesConfig_);
}

void WindowManagerService::MinimizeWindowsByLauncher(std::vector<uint32_t> windowIds, bool isAnimated,
    sptr<RSIWindowAnimationFinishedCallback>& finishCallback)
{
    if (!Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        WLOGFE("minimize windows by launcher permission denied!");
        return;
    }
    auto task = [this, windowIds, isAnimated, &finishCallback]() mutable {
        windowController_->MinimizeWindowsByLauncher(windowIds, isAnimated, finishCallback);
    };
    PostVoidSyncTask(task, "MinimizeWindowsByLauncher");
}

WMError WindowManagerService::UpdateAvoidAreaListener(uint32_t windowId, bool haveAvoidAreaListener)
{
    auto task = [this, windowId, haveAvoidAreaListener]() {
        sptr<WindowNode> node = windowRoot_->GetWindowNode(windowId);
        if (node == nullptr) {
            WLOGFE("get window node failed. win %{public}u", windowId);
            return WMError::WM_DO_NOTHING;
        }
        sptr<WindowNodeContainer> container = windowRoot_->GetWindowNodeContainer(node->GetDisplayId());
        if (container == nullptr) {
            WLOGFE("get container failed. win %{public}u display %{public}" PRIu64"", windowId, node->GetDisplayId());
            return WMError::WM_DO_NOTHING;
        }
        container->UpdateAvoidAreaListener(node, haveAvoidAreaListener);
        return WMError::WM_OK;
    };
    return PostSyncTask(task, "UpdateAvoidAreaListener");
}

void WindowManagerService::SetAnchorAndScale(int32_t x, int32_t y, float scale)
{
    auto task = [this, x, y, scale]() {
        windowController_->SetAnchorAndScale(x, y, scale);
    };
    PostAsyncTask(task, "SetAnchorAndScale");
}

void WindowManagerService::SetAnchorOffset(int32_t deltaX, int32_t deltaY)
{
    auto task = [this, deltaX, deltaY]() {
        windowController_->SetAnchorOffset(deltaX, deltaY);
    };
    PostAsyncTask(task, "SetAnchorOffset");
}

void WindowManagerService::OffWindowZoom()
{
    auto task = [this]() {
        windowController_->OffWindowZoom();
    };
    PostAsyncTask(task, "OffWindowZoom");
}

WMError WindowManagerService::UpdateRsTree(uint32_t windowId, bool isAdd)
{
    auto task = [this, windowId, isAdd]() {
        return windowRoot_->UpdateRsTree(windowId, isAdd);
    };
    return PostSyncTask(task, "UpdateRsTree");
}

void WindowManagerService::OnScreenshot(DisplayId displayId)
{
    auto task = [this, displayId]() {
        windowController_->OnScreenshot(displayId);
    };
    PostAsyncTask(task, "OnScreenshot");
}

WMError WindowManagerService::BindDialogTarget(uint32_t& windowId, sptr<IRemoteObject> targetToken)
{
    if (!Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        WLOGFE("bind dialog target permission denied!");
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    auto task = [this, &windowId, targetToken]() {
        return windowController_->BindDialogTarget(windowId, targetToken);
    };
    return PostSyncTask(task, "BindDialogTarget");
}

void WindowManagerService::HasPrivateWindow(DisplayId displayId, bool& hasPrivateWindow)
{
    auto task = [this, displayId, &hasPrivateWindow]() mutable {
        hasPrivateWindow = windowRoot_->HasPrivateWindow(displayId);
    };
    PostVoidSyncTask(task, "HasPrivateWindow");
    WLOGI("called %{public}u", hasPrivateWindow);
}

WMError WindowManagerService::SetGestureNavigationEnabled(bool enable)
{
    if (!Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        WLOGFE("permission denied!");
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    auto task = [this, enable]() {
        return windowRoot_->SetGestureNavigationEnabled(enable);
    };
    return PostSyncTask(task, "SetGestureNavigationEnabled");
}

void WindowInfoQueriedListener::HasPrivateWindow(DisplayId displayId, bool& hasPrivateWindow)
{
    WLOGI("called");
    WindowManagerService::GetInstance().HasPrivateWindow(displayId, hasPrivateWindow);
}

void WindowManagerService::SetMaximizeMode(MaximizeMode maximizeMode)
{
    maximizeMode_ = maximizeMode;
    int32_t storageMode = -1;
    if (PersistentStorage::HasKey("maximize_state", PersistentStorageType::MAXIMIZE_STATE)) {
        PersistentStorage::Get("maximize_state", storageMode, PersistentStorageType::MAXIMIZE_STATE);
        PersistentStorage::Delete("maximize_state", PersistentStorageType::MAXIMIZE_STATE);
    }
    PersistentStorage::Insert("maximize_state", static_cast<int32_t>(maximizeMode),
        PersistentStorageType::MAXIMIZE_STATE);
    WLOGI("globalMaximizeMode changed from %{public}d to %{public}d", storageMode, static_cast<int32_t>(maximizeMode));
}

MaximizeMode WindowManagerService::GetMaximizeMode()
{
    return maximizeMode_;
}

void WindowManagerService::GetFocusWindowInfo(FocusChangeInfo& focusInfo, DisplayId displayId)
{
    WLOGFD("Get Focus window info in wms");
    windowController_->GetFocusWindowInfo(focusInfo);
}

void WindowManagerService::InitRSUIDirector()
{
    rsUiDirector_ = RSUIDirector::Create();
    if (!rsUiDirector_) {
        TLOGE(WmsLogTag::WMS_SCB, "Failed to create RSUIDirector");
        return;
    }
    int32_t instanceId = INSTANCE_ID_UNDEFINED;
    bool useMultiInstance = false;
    RunIfRSClientMultiInstanceEnabled([&] {
        instanceId = 0;
        useMultiInstance = true;
        WindowInnerManager::GetInstance().SetRSUIDirector(rsUiDirector_);
    });
    rsUiDirector_->SetUITaskRunner([this](const std::function<void()>& task, uint32_t delay) {
            PostAsyncTask(task, "WindowManagerService:cacheGuard", delay);
        }, instanceId, useMultiInstance);
    rsUiDirector_->Init(false, useMultiInstance);
    TLOGI(WmsLogTag::WMS_SCB, "Create RSUIDirector: %{public}s",
          RSAdapterUtil::RSUIDirectorToStr(rsUiDirector_).c_str());
}
} // namespace Rosen
} // namespace OHOS
