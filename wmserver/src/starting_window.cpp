/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include <ability_manager_client.h>
#include <common/rs_common_def.h>
#include <display_manager_service_inner.h>
#include <hitrace_meter.h>
#include <res_config.h>
#include <transaction/rs_transaction.h>

#include "display_group_info.h"
#include "remote_animation.h"
#include "rs_adapter.h"
#include "starting_window.h"
#include "window_helper.h"
#include "window_inner_manager.h"
#include "window_manager_hilog.h"
#include "window_manager_service.h"
#include "window_manager_service_utils.h"
#include "window_system_effect.h"

namespace OHOS {
namespace Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "StartingWindow"};
constexpr int32_t DEFAULT_GIF_DELAY = 100;   //default delay time for gif
}

const std::map<OHOS::AppExecFwk::DisplayOrientation, Orientation> ABILITY_TO_WMS_ORIENTATION_MAP {
    {OHOS::AppExecFwk::DisplayOrientation::UNSPECIFIED, Orientation::UNSPECIFIED},
    {OHOS::AppExecFwk::DisplayOrientation::LANDSCAPE, Orientation::HORIZONTAL},
    {OHOS::AppExecFwk::DisplayOrientation::PORTRAIT, Orientation::VERTICAL},
    {OHOS::AppExecFwk::DisplayOrientation::FOLLOWRECENT, Orientation::LOCKED},
    {OHOS::AppExecFwk::DisplayOrientation::LANDSCAPE_INVERTED, Orientation::REVERSE_HORIZONTAL},
    {OHOS::AppExecFwk::DisplayOrientation::PORTRAIT_INVERTED, Orientation::REVERSE_VERTICAL},
    {OHOS::AppExecFwk::DisplayOrientation::AUTO_ROTATION, Orientation::SENSOR},
    {OHOS::AppExecFwk::DisplayOrientation::AUTO_ROTATION_LANDSCAPE, Orientation::SENSOR_HORIZONTAL},
    {OHOS::AppExecFwk::DisplayOrientation::AUTO_ROTATION_PORTRAIT, Orientation::SENSOR_VERTICAL},
    {OHOS::AppExecFwk::DisplayOrientation::AUTO_ROTATION_RESTRICTED, Orientation::AUTO_ROTATION_RESTRICTED},
    {OHOS::AppExecFwk::DisplayOrientation::AUTO_ROTATION_LANDSCAPE_RESTRICTED,
        Orientation::AUTO_ROTATION_LANDSCAPE_RESTRICTED},
    {OHOS::AppExecFwk::DisplayOrientation::AUTO_ROTATION_PORTRAIT_RESTRICTED,
        Orientation::AUTO_ROTATION_PORTRAIT_RESTRICTED},
    {OHOS::AppExecFwk::DisplayOrientation::LOCKED, Orientation::LOCKED},
};

std::recursive_mutex StartingWindow::mutex_;
WindowMode StartingWindow::defaultMode_ = WindowMode::WINDOW_MODE_FULLSCREEN;
bool StartingWindow::transAnimateEnable_ = true;
WindowUIType StartingWindow::windowUIType_ = WindowUIType::INVALID_WINDOW;
AnimationConfig StartingWindow::animationConfig_;
StartingWindowShowInfo StartingWindow::startingWindowShowInfo_;
std::atomic<bool> StartingWindow::startingWindowShowRunning_{false};
std::thread StartingWindow::startingWindowShowThread_;
std::mutex StartingWindow::firstFrameMutex_;
std::condition_variable StartingWindow::firstFrameCondition_;
std::atomic<bool> StartingWindow::firstFrameCompleted_ = false;

void StartingWindow::RegisterStartingWindowShowInfo(const sptr<WindowNode>& node, const Rect& rect,
    const std::shared_ptr<Rosen::StartingWindowPageDrawInfo>& info, float vpRatio)
{
    StartingWindowShowInfo startingWindowShowInfo { node, rect, info, vpRatio };
    std::array<std::weak_ptr<Rosen::ResourceInfo>, size_t(StartWindowResType::Count)> resources = {
        info->appIcon, info->illustration, info->branding, info->bgImage
    };
    auto now = std::chrono::steady_clock::now();
    for (size_t i = 0; i < resources.size(); ++i) {
        auto resInfo = resources[i].lock();
        auto& state = startingWindowShowInfo.resStates[i];
        if (!resInfo) {
            TLOGE(WmsLogTag::WMS_PATTERN, "null resource to record frame index.");
            state.frameCount = 0;
            continue;
        }
        state.delay = (!resInfo->delayTimes.empty()) ? resInfo->delayTimes : std::vector<int32_t>{ INT32_MAX };
        if (std::all_of(state.delay.begin(), state.delay.end(), [](int32_t delay) { return delay == 0; })) {
            for (auto& delay : state.delay) {
                TLOGD(WmsLogTag::WMS_PATTERN, "Invalid delay time: %{public}d", delay);
                delay = DEFAULT_GIF_DELAY;
            }
        }
        state.frameCount = (resInfo->pixelMaps.size() > 1) ? resInfo->pixelMaps.size() : 1;
        state.next = now;
    }
    startingWindowShowInfo_ = std::move(startingWindowShowInfo);
}

void StartingWindow::UnRegisterStartingWindowShowInfo()
{
    auto cleanResource = [](std::shared_ptr<Rosen::ResourceInfo>& resInfo) {
        if (resInfo) {
            for (auto& pixelMap : resInfo->pixelMaps) {
                pixelMap->FreePixelMap();
            }
            resInfo->pixelMaps.clear();
            resInfo->delayTimes.clear();
            resInfo.reset();
        }
    };
    if (startingWindowShowInfo_.info) {
        cleanResource(startingWindowShowInfo_.info->appIcon);
        cleanResource(startingWindowShowInfo_.info->bgImage);
        cleanResource(startingWindowShowInfo_.info->illustration);
        cleanResource(startingWindowShowInfo_.info->branding);
    }
    startingWindowShowInfo_.info.reset();
    TLOGE(WmsLogTag::WMS_PATTERN, "starting window draw resource cleaned.");
}

void StartingWindow::UpdateWindowShowInfo(StartingWindowShowInfo& startingWindowShowInfo, bool& needRedraw)
{
    for (size_t i = 0; i < startingWindowShowInfo.resStates.size(); ++i) {
        auto& state = startingWindowShowInfo.resStates[i];
        auto now = std::chrono::steady_clock::now();
        if (state.frameCount == 0 || now < state.next) {
            TLOGD(WmsLogTag::WMS_PATTERN, "null resource or not time yet");
            continue;
        }
        state.frameIdx = (state.frameIdx + 1) % state.frameCount;
        state.next = now + std::chrono::milliseconds(state.delay[state.frameIdx]);
        startingWindowShowInfo.frameIndex[i] = state.frameIdx;
        needRedraw = true;
    }
}

void StartingWindow::DrawStartingWindowShowInfo()
{
    startingWindowShowRunning_ = true;
    int32_t minDelay(DEFAULT_GIF_DELAY);
    for (const auto& state : startingWindowShowInfo_.resStates) {
        for (int32_t delay : state.delay) {
            if (delay > 0 && delay < minDelay) {
                minDelay = delay;
            }
        }
    }
    auto drawTask = [minDelay]() {
        bool firstDraw = true;
        while (startingWindowShowRunning_.load()) {
            bool needRedraw = false;
            UpdateWindowShowInfo(startingWindowShowInfo_, needRedraw);
            if (!needRedraw) {
                TLOGD(WmsLogTag::WMS_PATTERN, "no resource updated need to draw.");
                continue;
            }
            bool ret = SurfaceDraw::DrawCustomStartingWindow(
                startingWindowShowInfo_.node.promote()->startingWinSurfaceNode_,
                startingWindowShowInfo_.rect,
                startingWindowShowInfo_.info,
                startingWindowShowInfo_.vpRatio,
                startingWindowShowInfo_.frameIndex);
            if (firstDraw && !ret) {
                std::unique_lock<std::mutex> lock(firstFrameMutex_);
                startingWindowShowRunning_ = false;
                firstFrameCompleted_ = true;
                firstFrameCondition_.notify_one();
                TLOGE(WmsLogTag::WMS_PATTERN, "Failed to draw first frame of StartingWindow.");
                break;
            }
            if (firstDraw) {
                std::unique_lock<std::mutex> lock(firstFrameMutex_);
                firstFrameCompleted_ = true;
                firstFrameCondition_.notify_one();
                firstDraw = false;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(minDelay));
        }
    };
    startingWindowShowThread_ = std::thread(drawTask);
}

sptr<WindowNode> StartingWindow::CreateWindowNode(const sptr<WindowTransitionInfo>& info, uint32_t winId)
{
    sptr<WindowProperty> property = InitializeWindowProperty(info, winId);
    if (property == nullptr) {
        return nullptr;
    }

    sptr<WindowNode> node = new(std::nothrow) WindowNode(property);
    if (node == nullptr) {
        return nullptr;
    }

    node->stateMachine_.SetWindowId(winId);
    node->abilityToken_ = info->GetAbilityToken();
    node->SetWindowSizeLimits(info->GetWindowSizeLimits());
    node->abilityInfo_.missionId_ = info->GetMissionId();
    node->abilityInfo_.bundleName_ = info->GetBundleName();
    node->abilityInfo_.abilityName_ = info->GetAbilityName();
    uint32_t windowModeSupportType = WindowHelper::ConvertSupportModesToSupportType(info->GetWindowSupportModes());
    node->SetWindowModeSupportType(windowModeSupportType);

    if (CreateLeashAndStartingSurfaceNode(node) != WMError::WM_OK) {
        return nullptr;
    }
    node->stateMachine_.TransitionTo(WindowNodeState::STARTING_CREATED);
    return node;
}

sptr<WindowProperty> StartingWindow::InitializeWindowProperty(const sptr<WindowTransitionInfo>& info, uint32_t winId)
{
    sptr<WindowProperty> property = new(std::nothrow) WindowProperty();
    if (property == nullptr || info == nullptr) {
        return nullptr;
    }

    property->SetRequestRect(info->GetWindowRect());
    if (WindowHelper::IsValidWindowMode(info->GetWindowMode())) {
        property->SetWindowMode(info->GetWindowMode());
    } else {
        property->SetWindowMode(defaultMode_);
    }

    AppExecFwk::DisplayOrientation displayOrientation = info->GetOrientation();
    if (ABILITY_TO_WMS_ORIENTATION_MAP.count(displayOrientation) == 0) {
        WLOGFE("id:%{public}u Do not support this Orientation type", winId);
        return nullptr;
    }
    Orientation orientation = ABILITY_TO_WMS_ORIENTATION_MAP.at(displayOrientation);
    if (orientation < Orientation::BEGIN || orientation > Orientation::END) {
        WLOGFE("Set orientation from ability failed");
        return nullptr;
    }
    property->SetRequestedOrientation(orientation);

    property->SetDisplayId(info->GetDisplayId());
    property->SetWindowType(info->GetWindowType());

    ChangePropertyByApiVersion(info, orientation, property);
    property->SetApiCompatibleVersion(info->GetApiCompatibleVersion());
    if (info->GetShowFlagWhenLocked()) {
        if (property->GetApiCompatibleVersion() < 9 || info->isSystemCalling_) { // 9: api version.
            property->AddWindowFlag(WindowFlag::WINDOW_FLAG_SHOW_WHEN_LOCKED);
        } else {
            WLOGFW("Only API 9- or system calling support showing when locked.");
        }
    }
    property->SetWindowId(winId);
    return property;
}

void StartingWindow::ChangePropertyByApiVersion(const sptr<WindowTransitionInfo>& info,
    const Orientation orientation, sptr<WindowProperty>& property)
{
    // 10 ArkUI new framework support after API10
    if (info->GetApiCompatibleVersion() < 10) {
        auto displayInfo = DisplayGroupInfo::GetInstance().GetDisplayInfo(info->GetDisplayId());
        if (!(displayInfo && WmsUtils::IsExpectedRotatableWindow(orientation,
            displayInfo->GetDisplayOrientation(), property->GetWindowMode(), property->GetWindowFlags(), false))) {
            property->AddWindowFlag(WindowFlag::WINDOW_FLAG_NEED_AVOID);
        }
    } else {
        if (WindowHelper::IsMainWindow(property->GetWindowType())) {
            SystemBarProperty statusSystemBarProperty(true, 0x00FFFFFF, 0xFF000000);
            SystemBarProperty navigationSystemBarProperty(true, 0x00FFFFFF, 0xFF000000);
            property->SetSystemBarProperty(WindowType::WINDOW_TYPE_STATUS_BAR, statusSystemBarProperty);
            property->SetSystemBarProperty(WindowType::WINDOW_TYPE_NAVIGATION_BAR, navigationSystemBarProperty);
        }
    }
}

WMError StartingWindow::CreateLeashAndStartingSurfaceNode(sptr<WindowNode>& node)
{
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    rsSurfaceNodeConfig.SurfaceNodeName = "leashWindow" + std::to_string(node->GetWindowId());
    auto rsUIContext = node->GetRSUIContext();
    node->leashWinSurfaceNode_ = RSSurfaceNode::Create(
        rsSurfaceNodeConfig, RSSurfaceNodeType::LEASH_WINDOW_NODE, true, false, rsUIContext);
    if (node->leashWinSurfaceNode_ == nullptr) {
        TLOGE(WmsLogTag::WMS_STARTUP_PAGE, "create leashWinSurfaceNode failed");
        return WMError::WM_ERROR_NULLPTR;
    }
    RSAdapterUtil::SetSkipCheckInMultiInstance(node->leashWinSurfaceNode_, true);
    TLOGD(WmsLogTag::WMS_SCB, "Create RSSurfaceNode: %{public}s, name: %{public}s",
          RSAdapterUtil::RSNodeToStr(node->leashWinSurfaceNode_).c_str(),
          rsSurfaceNodeConfig.SurfaceNodeName.c_str());

    rsSurfaceNodeConfig.SurfaceNodeName = "startingWindow" + std::to_string(node->GetWindowId());
    node->startingWinSurfaceNode_ = RSSurfaceNode::Create(
        rsSurfaceNodeConfig, RSSurfaceNodeType::STARTING_WINDOW_NODE, true, false, rsUIContext);
    if (node->startingWinSurfaceNode_ == nullptr) {
        TLOGE(WmsLogTag::WMS_STARTUP_PAGE, "create startingWinSurfaceNode failed");
        node->leashWinSurfaceNode_ = nullptr;
        return WMError::WM_ERROR_NULLPTR;
    }
    RSAdapterUtil::SetSkipCheckInMultiInstance(node->startingWinSurfaceNode_, true);
    TLOGD(WmsLogTag::WMS_SCB, "Create RSSurfaceNode: %{public}s, name: %{public}s",
          RSAdapterUtil::RSNodeToStr(node->startingWinSurfaceNode_).c_str(),
          rsSurfaceNodeConfig.SurfaceNodeName.c_str());
    TLOGI(WmsLogTag::WMS_STARTUP_PAGE,
        "Create leashWinSurfaceNode and startingWinSurfaceNode success with id:%{public}u!",
        node->GetWindowId());
    return WMError::WM_OK;
}

WMError StartingWindow::DrawStartingWindow(sptr<WindowNode>& node,
    std::shared_ptr<Media::PixelMap> pixelMap, uint32_t bkgColor, bool isColdStart)
{
    if (node == nullptr) {
        return WMError::WM_ERROR_NULLPTR;
    }
    // using snapshot to support hot start since node destroy when hide
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "wms:DrawStartingWindow(%u)", node->GetWindowId());
    Rect rect = node->GetWindowRect();
    if (RemoteAnimation::CheckRemoteAnimationEnabled(node->GetDisplayId()) && node->leashWinSurfaceNode_) {
        // hides this node until RSProxyNode send valid context alpha/matrix
        node->leashWinSurfaceNode_->ResetContextAlpha();
    }
    if (!isColdStart) {
        return WMError::WM_OK;
    }
    if (node->startingWinSurfaceNode_ == nullptr) {
        TLOGE(WmsLogTag::WMS_STARTUP_PAGE, "no starting Window SurfaceNode!");
        return WMError::WM_ERROR_NULLPTR;
    }
    auto staringWindowPageDrawInfo = GetCustomStartingWindowInfo(node, GetBundleManager());
    if (staringWindowPageDrawInfo) {
        if (DrawStartingWindow(staringWindowPageDrawInfo, node, rect)  ==  WMError::WM_OK) {
            return WMError::WM_OK;
        }
    }
    // set window effect
    WindowSystemEffect::SetWindowEffect(node);
    if (pixelMap == nullptr) {
        SurfaceDraw::DrawColor(node->startingWinSurfaceNode_, rect.width_, rect.height_, bkgColor);
        return WMError::WM_OK;
    }

    TLOGD(WmsLogTag::WMS_STARTUP_PAGE, "draw background in sperate");
    SurfaceDraw::DrawImageRect(node->startingWinSurfaceNode_, rect, pixelMap, bkgColor);
    return WMError::WM_OK;
}

WMError StartingWindow::SetStartingWindowAnimation(wptr<WindowNode> weak)
{
    auto weakNode = weak.promote();
    if (weakNode == nullptr || !weakNode->startingWinSurfaceNode_) {
        TLOGE(WmsLogTag::WMS_STARTUP_PAGE, "windowNode or startingWinSurfaceNode_ is nullptr");
        return WMError::WM_ERROR_NULLPTR;
    }
    StartAsyncTraceArgs(HITRACE_TAG_WINDOW_MANAGER, static_cast<int32_t>(TraceTaskId::START_WINDOW_ANIMATION),
        "StartingWindowAnimate(%u)", weakNode->GetWindowId());
    weakNode->startingWinSurfaceNode_->SetAlpha(animationConfig_.startWinAnimationConfig_.opacityStart_);
    auto execute = [weak]() {
        auto weakNode = weak.promote();
        if (weakNode == nullptr) {
            TLOGNE(WmsLogTag::WMS_STARTUP_PAGE, "windowNode is nullptr");
            return;
        }
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "StartingWindow:ExecuteAnimate(%d)",
            weakNode->GetWindowId());
        weakNode->startingWinSurfaceNode_->SetAlpha(animationConfig_.startWinAnimationConfig_.opacityEnd_);
    };

    auto finish = [weak]() {
        auto weakNode = weak.promote();
        if (weakNode == nullptr || weakNode->leashWinSurfaceNode_ == nullptr) {
            TLOGE(WmsLogTag::WMS_STARTUP_PAGE, "windowNode or leashWinSurfaceNode_ is nullptr");
            return;
        }
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "StartingWindow:AnimateFinish(%d)",
            weakNode->GetWindowId());
        TLOGNI(WmsLogTag::WMS_STARTUP_PAGE, "StartingWindow::Replace surfaceNode, id: %{public}u",
            weakNode->GetWindowId());
        weakNode->leashWinSurfaceNode_->RemoveChild(weakNode->startingWinSurfaceNode_);
        weakNode->startingWinSurfaceNode_ = nullptr;
        RSTransactionAdapter::FlushImplicitTransaction(weakNode->leashWinSurfaceNode_);
        FinishAsyncTraceArgs(HITRACE_TAG_WINDOW_MANAGER, static_cast<int32_t>(TraceTaskId::START_WINDOW_ANIMATION),
            "StartingWindowAnimate(%u)", weakNode->GetWindowId());
    };
    auto rsUIContext = weakNode->startingWinSurfaceNode_->GetRSUIContext();
    RSNode::Animate(rsUIContext, animationConfig_.startWinAnimationConfig_.timingProtocol_,
                    animationConfig_.startWinAnimationConfig_.timingCurve_, execute, finish);
    RSTransactionAdapter::FlushImplicitTransaction(rsUIContext);
    FinishTrace(HITRACE_TAG_WINDOW_MANAGER);
    return WMError::WM_OK;
}

void StartingWindow::HandleClientWindowCreate(sptr<WindowNode>& node, sptr<IWindow>& window,
    uint32_t& windowId, const std::shared_ptr<RSSurfaceNode>& surfaceNode, sptr<WindowProperty>& property,
    int32_t pid, int32_t uid)
{
    if (node == nullptr) {
        TLOGE(WmsLogTag::WMS_STARTUP_PAGE, "node is nullptr");
        return;
    }
    node->surfaceNode_ = surfaceNode;
    node->SetWindowToken(window);
    node->SetCallingPid(pid);
    node->SetCallingUid(uid);
    windowId = node->GetWindowId();
    // test
    node->stateMachine_.SetWindowId(windowId);
    node->stateMachine_.SetWindowType(property->GetWindowType());
    TLOGI(WmsLogTag::WMS_STARTUP_PAGE, "after set Id:%{public}u, "
        "requestRect:[%{public}d, %{public}d, %{public}u, %{public}u]",
        node->GetWindowId(), node->GetRequestRect().posX_, node->GetRequestRect().posY_,
        node->GetRequestRect().width_, node->GetRequestRect().height_);
    // Register FirstFrame Callback to rs, replace startwin
    wptr<WindowNode> weak = node;
    auto firstFrameCompleteCallback = [weak]() {
        auto task = [weak]() {
            FinishAsyncTraceArgs(HITRACE_TAG_WINDOW_MANAGER, static_cast<int32_t>(TraceTaskId::STARTING_WINDOW),
                "wms:async:ShowStartingWindow");
            auto weakNode = weak.promote();
            if (weakNode == nullptr || weakNode->leashWinSurfaceNode_ == nullptr) {
                TLOGNE(WmsLogTag::WMS_STARTUP_PAGE, "windowNode or leashWinSurfaceNode_ is nullptr");
                return;
            }
            TLOGNI(WmsLogTag::WMS_STARTUP_PAGE, "StartingWindow::FirstFrameCallback come, id: %{public}u",
                weakNode->GetWindowId());
            if (transAnimateEnable_) {
                SetStartingWindowAnimation(weakNode);
            } else {
                weakNode->leashWinSurfaceNode_->RemoveChild(weakNode->startingWinSurfaceNode_);
                weakNode->startingWinSurfaceNode_ = nullptr;
                RSTransactionAdapter::FlushImplicitTransaction(weakNode->leashWinSurfaceNode_);
                TLOGNI(WmsLogTag::WMS_STARTUP_PAGE, "StartingWindow::Replace surfaceNode, id: %{public}u",
                    weakNode->GetWindowId());
            }
            WindowInnerManager::GetInstance().CompleteFirstFrameDrawing(weakNode);
            weakNode->firstFrameAvailable_ = true;
            startingWindowShowRunning_ = false;
            if (startingWindowShowThread_.joinable()) {
                startingWindowShowThread_.join();
            }
            UnRegisterStartingWindowShowInfo();
        };
        WindowManagerService::GetInstance().PostAsyncTask(task, "firstFrameCompleteCallback");
    };
    node->surfaceNode_->SetBufferAvailableCallback(firstFrameCompleteCallback);
    RSTransactionAdapter::FlushImplicitTransaction(node->surfaceNode_);
}

void StartingWindow::ReleaseStartWinSurfaceNode(sptr<WindowNode>& node)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (!node->leashWinSurfaceNode_) {
        TLOGI(WmsLogTag::WMS_STARTUP_PAGE, "cannot release leashwindow since leash is null, id:%{public}u",
            node->GetWindowId());
        return;
    }
    if (startingWindowShowRunning_.load()) {
        startingWindowShowRunning_ = false;
        if (startingWindowShowThread_.joinable()) {
            startingWindowShowThread_.join();
        }
        UnRegisterStartingWindowShowInfo();
    }
    node->leashWinSurfaceNode_->RemoveChild(node->startingWinSurfaceNode_);
    node->leashWinSurfaceNode_->RemoveChild(node->closeWinSurfaceNode_);
    node->leashWinSurfaceNode_->RemoveChild(node->surfaceNode_);
    RSTransactionAdapter::FlushImplicitTransaction(node->leashWinSurfaceNode_);
    node->leashWinSurfaceNode_ = nullptr;
    node->startingWinSurfaceNode_ = nullptr;
    node->closeWinSurfaceNode_ = nullptr;
    TLOGI(WmsLogTag::WMS_STARTUP_PAGE, "Release startwindow surfaceNode end id: %{public}u, "
        "[leashWinSurface]: use_count: %{public}ld, [startWinSurface]: use_count: %{public}ld ",
        node->GetWindowId(), node->leashWinSurfaceNode_.use_count(),
        node->startingWinSurfaceNode_.use_count());
}

bool StartingWindow::IsWindowFollowParent(WindowType type)
{
    if (windowUIType_ != WindowUIType::PHONE_WINDOW) {
        return false;
    }
    return WindowHelper::IsWindowFollowParent(type);
}

void StartingWindow::AddNodeOnRSTree(sptr<WindowNode>& node, bool isMultiDisplay)
{
    auto updateRSTreeFunc = [&node, isMultiDisplay]() {
        UpdateRSTree(node, isMultiDisplay);
    };
    wptr<WindowNode> weakNode = node;
    auto finishCallBack = [weakNode]() {
        auto weak = weakNode.promote();
        if (weak == nullptr) {
            return;
        }
        auto winRect = weak->GetWindowRect();
        TLOGI(WmsLogTag::WMS_STARTUP_PAGE,
            "before setBounds windowRect: %{public}d, %{public}d, %{public}d, %{public}d",
            winRect.posX_, winRect.posY_, winRect.width_, winRect.height_);
        if (weak->leashWinSurfaceNode_) {
            weak->leashWinSurfaceNode_->SetBounds(winRect.posX_, winRect.posY_, winRect.width_, winRect.height_);
            weak->leashWinSurfaceNode_->SetAnimationFinished();
        }
        RSTransactionAdapter::FlushImplicitTransaction(weak->leashWinSurfaceNode_);
    };
    if (!RemoteAnimation::CheckAnimationController()) {
        std::shared_ptr<RSUIContext> rsUIContext;
        if (node->leashWinSurfaceNode_) {
            rsUIContext = node->leashWinSurfaceNode_->GetRSUIContext();
        }
        RSNode::Animate(rsUIContext, animationConfig_.windowAnimationConfig_.animationTiming_.timingProtocol_,
                        animationConfig_.windowAnimationConfig_.animationTiming_.timingCurve_,
                        updateRSTreeFunc, finishCallBack);
    } else {
        // add or remove window without animation
        updateRSTreeFunc();
    }
}

void StartingWindow::UpdateRSTree(sptr<WindowNode>& node, bool isMultiDisplay)
{
    auto& dms = DisplayManagerServiceInner::GetInstance();
    DisplayId displayId = node->GetDisplayId();
    if (!node->surfaceNode_) { // cold start
        if (!WindowHelper::IsMainWindow(node->GetWindowType())) {
            TLOGE(WmsLogTag::WMS_MAIN, "window id: %{public}d type: %{public}u is not Main Window!",
                node->GetWindowId(), static_cast<uint32_t>(node->GetWindowType()));
        }
        dms.UpdateRSTree(displayId, displayId, node->leashWinSurfaceNode_, true, isMultiDisplay);
        node->leashWinSurfaceNode_->AddChild(node->startingWinSurfaceNode_, -1);
        TLOGD(WmsLogTag::WMS_MAIN, "Add startingWinSurfaceNode_ to leashWinSurfaceNode_ end during cold start");
        return;
    }
    // hot start
    const auto& displayIdVec = node->GetShowingDisplays();
    for (auto& shownDisplayId : displayIdVec) {
        if (node->leashWinSurfaceNode_) { // to app
            dms.UpdateRSTree(shownDisplayId, shownDisplayId, node->leashWinSurfaceNode_, true, isMultiDisplay);
        } else { // to launcher
            dms.UpdateRSTree(shownDisplayId, shownDisplayId, node->surfaceNode_, true, isMultiDisplay);
        }
        for (auto& child : node->children_) {
            if (IsWindowFollowParent(child->GetWindowType())) {
                continue;
            }
            if (child->currentVisibility_) {
                dms.UpdateRSTree(shownDisplayId, shownDisplayId, child->surfaceNode_, true, isMultiDisplay);
            }
        }
    }
    TLOGD(WmsLogTag::WMS_MAIN, "Update RsTree with hot start");
}

void StartingWindow::SetDefaultWindowMode(WindowMode defaultMode)
{
    defaultMode_ = defaultMode;
}

void StartingWindow::SetAnimationConfig(AnimationConfig config)
{
    animationConfig_ = config;
}

sptr<AppExecFwk::IBundleMgr> StartingWindow::GetBundleManager()
{
    auto systemAbilityMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityMgr == nullptr) {
        TLOGE(WmsLogTag::WMS_PATTERN, "Failed to get SystemAbilityManager.");
        return nullptr;
    }
    auto bmsProxy = systemAbilityMgr->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (bmsProxy == nullptr) {
        TLOGE(WmsLogTag::WMS_PATTERN, "Failed to get BundleManagerService.");
        return nullptr;
    }
    return iface_cast<AppExecFwk::IBundleMgr>(bmsProxy);
}

std::shared_ptr<AppExecFwk::AbilityInfo> StartingWindow::GetAbilityInfoFromBMS(const sptr<WindowNode>& node,
    const sptr<AppExecFwk::IBundleMgr>& bundleMgr)
{
    if (node == nullptr || bundleMgr == nullptr) {
        TLOGE(WmsLogTag::WMS_PATTERN, "node or bundleMgr is nullptr.");
        return nullptr;
    }
    AAFwk::Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    want.SetElementName("", node->abilityInfo_.bundleName_, node->abilityInfo_.abilityName_, "");
    if (!bundleMgr->QueryAbilityInfo(
        want, AppExecFwk::GET_ABILITY_INFO_DEFAULT, AppExecFwk::Constants::ANY_USERID, abilityInfo)) {
        TLOGE(WmsLogTag::WMS_PATTERN, "Get ability info from BMS failed!");
        return nullptr;
    }
    return std::make_shared<AppExecFwk::AbilityInfo>(std::move(abilityInfo));
}

std::shared_ptr<Global::Resource::ResourceManager> StartingWindow::CreateResourceManager(
    const std::shared_ptr<AppExecFwk::AbilityInfo>& abilityInfo)
{
    if (abilityInfo == nullptr) {
        TLOGE(WmsLogTag::WMS_PATTERN, "abilityInfo is nullptr.");
        return nullptr;
    }
    std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
    if (resConfig == nullptr) {
        TLOGE(WmsLogTag::WMS_PATTERN, "resConfig is nullptr.");
        return nullptr;
    }
    std::shared_ptr<Global::Resource::ResourceManager> resourceMgr(Global::Resource::CreateResourceManager(false));
    if (resourceMgr == nullptr) {
        TLOGE(WmsLogTag::WMS_PATTERN, "resourceMgr is nullptr.");
        return nullptr;
    }

    std::string loadPath = abilityInfo->hapPath.empty() ? abilityInfo->resourcePath : abilityInfo->hapPath;
    if (!resourceMgr->AddResource(loadPath.c_str(), Global::Resource::SELECT_COLOR | Global::Resource::SELECT_MEDIA)) {
        TLOGE(WmsLogTag::WMS_PATTERN, "Add resource %{private}s failed.", loadPath.c_str());
    }
    return resourceMgr;
}

std::shared_ptr<Rosen::ResourceInfo> StartingWindow::GetPixelMapListInfo(uint32_t mediaDataId,
    const std::shared_ptr<Global::Resource::ResourceManager>& resourceMgr,
    const std::shared_ptr<AppExecFwk::AbilityInfo>& abilityInfo)
{
    if (mediaDataId <= 0 || resourceMgr == nullptr || abilityInfo == nullptr) {
        TLOGE(WmsLogTag::WMS_PATTERN, "invalid mediaDataId or null resourceMgr and abilityInfo.");
        return nullptr;
    }
    Media::SourceOptions opts;
    uint32_t errorCode = 0;
    std::unique_ptr<Media::ImageSource> imageSource;
    if (!abilityInfo->hapPath.empty()) {
        size_t len = 0;
        std::unique_ptr<uint8_t[]> dataOut;
        if (resourceMgr->GetMediaDataById(mediaDataId, len, dataOut) != Global::Resource::RState::SUCCESS) {
            return nullptr;
        }
        imageSource = Media::ImageSource::CreateImageSource(dataOut.get(), len, opts, errorCode);
    } else {
        std::string dataPath;
        if (resourceMgr->GetMediaById(mediaDataId, dataPath) != Global::Resource::RState::SUCCESS) {
            return nullptr;
        }
        imageSource = Media::ImageSource::CreateImageSource(dataPath, opts, errorCode);
    }
    if (errorCode != 0 || imageSource == nullptr) {
        TLOGE(WmsLogTag::WMS_PATTERN, "imageSource failed, id %{public}u err %{public}u", mediaDataId, errorCode);
        return nullptr;
    }
    Media::DecodeOptions decodeOpts;
    auto pixelMapList = imageSource->CreatePixelMapList(decodeOpts, errorCode);
    if (errorCode != 0) {
        TLOGE(WmsLogTag::WMS_PATTERN, "pixelMapList failed, id %{public}u err %{public}u", mediaDataId, errorCode);
        return nullptr;
    }
    auto info = std::make_shared<Rosen::ResourceInfo>();
    std::transform(pixelMapList->begin(), pixelMapList->end(), std::back_inserter(info->pixelMaps),
                   [](auto& ptr) { return std::move(ptr); });
    if (info->pixelMaps.size() > 1) {     // multi frames
        auto delaytimes = imageSource->GetDelayTime(errorCode);
        if (errorCode != 0) {
            TLOGE(WmsLogTag::WMS_PATTERN, "delaytimes failed, id %{public}u err %{public}u", mediaDataId, errorCode);
            return nullptr;
        }
        info->delayTimes = std::move(*delaytimes);
    }
    return info;
}

std::shared_ptr<Rosen::StartingWindowPageDrawInfo> StartingWindow::GetCustomStartingWindowInfo(
    const sptr<WindowNode>& node, const sptr<AppExecFwk::IBundleMgr>& bundleMgr)
{
    if (node == nullptr || bundleMgr == nullptr) {
        TLOGE(WmsLogTag::WMS_PATTERN, "node or bundleMgr is nullptr.");
        return nullptr;
    }
    auto abilityInfo = GetAbilityInfoFromBMS(node, bundleMgr);
    if (abilityInfo == nullptr) {
        TLOGE(WmsLogTag::WMS_PATTERN, "Failed to retrieve abilityinfo from BMS");
        return nullptr;
    }
    auto resourceMgr = CreateResourceManager(abilityInfo);
    if (resourceMgr == nullptr) {
        TLOGE(WmsLogTag::WMS_PATTERN, "Failed to create resource manager");
        return nullptr;
    }
    return DoGetCustomStartingWindowInfo(abilityInfo, resourceMgr);
}

std::shared_ptr<Rosen::StartingWindowPageDrawInfo> StartingWindow::DoGetCustomStartingWindowInfo(
    const std::shared_ptr<AppExecFwk::AbilityInfo>& abilityInfo,
    const std::shared_ptr<Global::Resource::ResourceManager>& resourceMgr)
{
    if (resourceMgr == nullptr || abilityInfo == nullptr) {
        TLOGE(WmsLogTag::WMS_PATTERN, "resourMgr or abilityInfo is nullptr.");
        return nullptr;
    }

    const auto& startWindowRes = abilityInfo->startWindowResource;
    auto loadPixelMap = [&](uint32_t resId) {
        return GetPixelMapListInfo(resId, resourceMgr, abilityInfo);
    };

    auto info = std::make_shared<Rosen::StartingWindowPageDrawInfo>();
    if (resourceMgr->GetColorById(startWindowRes.startWindowBackgroundColorId, info->bgColor) ==
        Global::Resource::RState::SUCCESS) {
        info->bgImage = loadPixelMap(startWindowRes.startWindowBackgroundImageId);
        info->branding = loadPixelMap(startWindowRes.startWindowBrandingImageId);
        info->startWindowBackgroundImageFit = startWindowRes.startWindowBackgroundImageFit;
        info->appIcon = loadPixelMap(startWindowRes.startWindowAppIconId);
        if (info->appIcon == nullptr) {
            info->illustration = loadPixelMap(startWindowRes.startWindowIllustrationId);
        }
        return info;
    }
    TLOGE(WmsLogTag::WMS_PATTERN, "Failed to load custom startingwindow color.");
    return nullptr;
}

WMError StartingWindow::DrawStartingWindow(const std::shared_ptr<Rosen::StartingWindowPageDrawInfo>& info,
    const sptr<WindowNode>& node, const Rect& rect)
{
    if (startingWindowShowRunning_.load()) {
        startingWindowShowRunning_ = false;
        if (startingWindowShowThread_.joinable()) {
            startingWindowShowThread_.join();
        }
        firstFrameCompleted_ = false;
        UnRegisterStartingWindowShowInfo();
    }

    if (info == nullptr) {
        TLOGE(WmsLogTag::WMS_PATTERN, "startingWindowPageDrawInfo is nullptr.");
        return WMError::WM_ERROR_NULLPTR;
    }

    const float vpRatio = DisplayGroupInfo::GetInstance().GetDisplayVirtualPixelRatio(node->GetDisplayId());
    RegisterStartingWindowShowInfo(node, rect, info, vpRatio);
    const auto& resInfo = startingWindowShowInfo_.info;
    if (resInfo && !resInfo->appIcon && !resInfo->bgImage && !resInfo->branding && !resInfo->illustration) {
        if (!(SurfaceDraw::DrawCustomStartingWindow(node->startingWinSurfaceNode_, rect, resInfo, vpRatio,
            startingWindowShowInfo_.frameIndex))) {
            TLOGE(WmsLogTag::WMS_PATTERN, "Failed to draw Custom starting window.");
            return WMError::WM_ERROR_INVALID_PARAM;
        }
        return WMError::WM_OK;
    }

    DrawStartingWindowShowInfo();
    std::unique_lock<std::mutex> lock(firstFrameMutex_);
    firstFrameCondition_.wait(lock, []() { return firstFrameCompleted_ == true; });
    if (!startingWindowShowRunning_.load()) {
        TLOGE(WmsLogTag::WMS_PATTERN, "Failed to start Custom starting window show.");
        if (startingWindowShowThread_.joinable()) {
            startingWindowShowThread_.join();
        }
        UnRegisterStartingWindowShowInfo();
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    return WMError::WM_OK;
}
} // Rosen
} // OHOS