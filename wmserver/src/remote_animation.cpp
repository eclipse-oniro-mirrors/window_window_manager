/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "remote_animation.h"

#include <ability_manager_client.h>
#include <common/rs_rect.h>
#include <hisysevent.h>
#include <hitrace_meter.h>
#include <string>
#include <transaction/rs_transaction.h>

#include "display_group_info.h"
#include "minimize_app.h"
#include "parameters.h"
#include "rs_adapter.h"
#include "starting_window.h"
#include "surface_draw.h"
#include "window_helper.h"
#include "window_inner_manager.h"
#include "window_manager_hilog.h"
#include "window_manager_service_utils.h"
#include "window_system_effect.h"
#include "zidl/ressched_report.h"
#ifdef SOC_PERF_ENABLE
#include "socperf_client.h"
#endif

namespace OHOS {
namespace Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "RemoteAnimation"};
const std::string ANIMATION_TIME_OUT_TASK = "remote_animation_time_out_task";
constexpr int64_t ANIMATION_TIME_OUT_MILLISECONDS = 3000; // 3000 is max time
}
bool RemoteAnimation::isRemoteAnimationEnable_ = true;
std::atomic<uint32_t> RemoteAnimation::allocationId_ = 0;
sptr<RSIWindowAnimationController> RemoteAnimation::windowAnimationController_ = nullptr;
std::weak_ptr<AppExecFwk::EventHandler> RemoteAnimation::wmsTaskHandler_;
wptr<WindowRoot> RemoteAnimation::windowRoot_;
bool RemoteAnimation::animationFirst_ = false;
wptr<WindowController> RemoteAnimation::windowController_ = nullptr;

std::map<TransitionReason, TransitionEvent> eventMap_ = {
    {TransitionReason::CLOSE, TransitionEvent::CLOSE},
    {TransitionReason::MINIMIZE, TransitionEvent::MINIMIZE},
    {TransitionReason::BACK_TRANSITION, TransitionEvent::BACK_TRANSITION},
    {TransitionReason::CLOSE_BUTTON, TransitionEvent::CLOSE_BUTTON},
    {TransitionReason::BACKGROUND_TRANSITION, TransitionEvent::BACKGROUND_TRANSITION}
};

void RemoteAnimation::SetAnimationFirst(bool animationFirst)
{
    animationFirst_ = animationFirst;
    WLOGFI("RSWindowAnimation:animationFirst: %{public}u!", static_cast<uint32_t>(animationFirst_));
}

bool RemoteAnimation::IsRemoteAnimationEnabledAndFirst(DisplayId displayId)
{
    return animationFirst_ && CheckRemoteAnimationEnabled(displayId);
}

WMError RemoteAnimation::SetWindowAnimationController(const sptr<RSIWindowAnimationController>& controller)
{
    WLOGFI("RSWindowAnimation: set window animation controller!");
    if (!isRemoteAnimationEnable_) {
        WLOGFE("RSWindowAnimation: failed to set window animation controller, remote animation is not enabled");
        return WMError::WM_ERROR_NO_REMOTE_ANIMATION;
    }
    if (controller == nullptr) {
        WLOGFE("RSWindowAnimation: failed to set window animation controller, controller is null!");
        return WMError::WM_ERROR_NULLPTR;
    }

    if (windowAnimationController_ != nullptr) {
        WLOGFI("RSWindowAnimation: maybe user switch!");
    }

    windowAnimationController_ = controller;
    auto winRoot = windowRoot_.promote();
    if (winRoot) {
        auto wallpaperNode = winRoot->FindWallpaperWindow();
        if (wallpaperNode) {
            NotifyAnimationUpdateWallpaper(wallpaperNode);
        } else {
            WLOGFW("Cannot find wallpaper window!");
        }
    }
    return WMError::WM_OK;
}

void RemoteAnimation::SetMainTaskHandler(std::shared_ptr<AppExecFwk::EventHandler> handler)
{
    wmsTaskHandler_ = handler;
}

void RemoteAnimation::SetWindowControllerAndRoot(const sptr<WindowController>& windowController,
    const sptr<WindowRoot>& windowRoot)
{
    windowController_ = windowController;
    windowRoot_ = windowRoot;
}

bool RemoteAnimation::CheckAnimationController()
{
    if (windowAnimationController_ == nullptr) {
        WLOGFD("RSWindowAnimation: windowAnimationController_ null!");
        return false;
    }
    return true;
}

bool RemoteAnimation::CheckRemoteAnimationEnabled(DisplayId displayId)
{
    // When the screen is locked, remote animation cannot take effect because the launcher is frozen.
    auto winRoot = windowRoot_.promote();
    if (winRoot == nullptr) {
        return false;
    }
    auto container = winRoot->GetOrCreateWindowNodeContainer(displayId);
    if (container == nullptr || container->IsScreenLocked()) {
        return false;
    }
    return CheckAnimationController();
}

bool RemoteAnimation::CheckTransition(sptr<WindowTransitionInfo> srcInfo, const sptr<WindowNode>& srcNode,
    sptr<WindowTransitionInfo> dstInfo, const sptr<WindowNode>& dstNode)
{
    if (srcNode == nullptr && dstNode == nullptr) {
        WLOGFE("RSWindowAnimation: srcNode and dstNode are nullptr");
        return false;
    }

    if (srcNode != nullptr && !srcNode->leashWinSurfaceNode_ && !srcNode->surfaceNode_) {
        WLOGFE("RSWindowAnimation: srcNode has no surface, winId: %{public}u", srcNode->GetWindowId());
        return false;
    }

    if (dstNode != nullptr && !dstNode->leashWinSurfaceNode_ && !dstNode->surfaceNode_) {
        WLOGFE("RSWindowAnimation: dstNode has no surface, winId: %{public}u", dstNode->GetWindowId());
        return false;
    }

    // check support window mode when one app starts another app
    if ((dstNode != nullptr && dstInfo != nullptr) &&
        !WindowHelper::CheckSupportWindowMode(dstNode->GetWindowMode(), dstNode->GetWindowModeSupportType(), dstInfo)) {
        WLOGFE("RSWindowAnimation: the mode of dstNode isn't supported, winId: %{public}u, mode: %{public}u, "
            "windowModeSupportType: %{public}u", dstNode->GetWindowId(), dstNode->GetWindowMode(),
            dstNode->GetWindowModeSupportType());
        return false;
    }

    auto node = (dstNode != nullptr ? dstNode : srcNode);
    return CheckRemoteAnimationEnabled(node->GetDisplayId());
}

void RemoteAnimation::OnRemoteDie(const sptr<IRemoteObject>& remoteObject)
{
    WLOGFI("RSWindowAnimation: OnRemoteDie!");
    if (windowAnimationController_ != nullptr && windowAnimationController_->AsObject() == remoteObject) {
        windowAnimationController_ = nullptr;
    }
    if (animationFirst_) {
        CallbackTimeOutProcess();
    }
}

static void GetAndDrawSnapShot(const sptr<WindowNode>& srcNode)
{
    if (srcNode == nullptr || srcNode->leashWinSurfaceNode_ == nullptr) {
        TLOGD(WmsLogTag::WMS_STARTUP_PAGE, "srcNode or srcNode->leashWinSurfaceNode_ is empty");
        return;
    }
    if (srcNode->firstFrameAvailable_) {
        std::shared_ptr<Media::PixelMap> pixelMap;
        bool snapSucc = SurfaceDraw::GetSurfaceSnapshot(srcNode->surfaceNode_, pixelMap, SNAPSHOT_TIMEOUT_MS, 1.0, 1.0);
        if (!snapSucc) {
            // need to draw starting window when get pixelmap failed
            TLOGE(WmsLogTag::WMS_STARTUP_PAGE, "get surfaceSnapshot failed for window:%{public}u",
                srcNode->GetWindowId());
            return;
        }
        WindowInnerManager::GetInstance().UpdateMissionSnapShot(srcNode, pixelMap);
        struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
        rsSurfaceNodeConfig.SurfaceNodeName = "closeWin" + std::to_string(srcNode->GetWindowId());
        RSTransactionAdapter rsTransAdapter(srcNode->leashWinSurfaceNode_);
        srcNode->closeWinSurfaceNode_ = RSSurfaceNode::Create(rsSurfaceNodeConfig,
            RSSurfaceNodeType::STARTING_WINDOW_NODE, true, false, rsTransAdapter.GetRSUIContext());
        RSAdapterUtil::SetSkipCheckInMultiInstance(srcNode->closeWinSurfaceNode_, true);
        TLOGD(WmsLogTag::WMS_SCB,
              "Create RSSurfaceNode: %{public}s", RSAdapterUtil::RSNodeToStr(srcNode->closeWinSurfaceNode_).c_str());
        auto rect = srcNode->GetWindowRect();
        srcNode->closeWinSurfaceNode_->SetBounds(0, 0, rect.width_, rect.height_);
        SurfaceDraw::DrawImageRect(srcNode->closeWinSurfaceNode_, srcNode->GetWindowRect(),
            pixelMap, 0x00ffffff, true);
        srcNode->leashWinSurfaceNode_->RemoveChild(srcNode->surfaceNode_);
        srcNode->leashWinSurfaceNode_->AddChild(srcNode->closeWinSurfaceNode_, -1);
        rsTransAdapter.FlushImplicitTransaction();
        TLOGI(WmsLogTag::WMS_STARTUP_PAGE, "Draw surface snapshot in starting window for window:%{public}u",
            srcNode->GetWindowId());
    } else if (srcNode->surfaceNode_) {
        srcNode->surfaceNode_->SetIsNotifyUIBufferAvailable(true);
        TLOGI(WmsLogTag::WMS_STARTUP_PAGE, "Draw startingWindow in starting window for window:%{public}u",
            srcNode->GetWindowId());
    }
}

TransitionEvent RemoteAnimation::GetTransitionEvent(sptr<WindowTransitionInfo> srcInfo,
    sptr<WindowTransitionInfo> dstInfo, const sptr<WindowNode>& srcNode, const sptr<WindowNode>& dstNode)
{
    auto transitionReason = srcInfo->GetTransitionReason(); // src reason same as dst reason
    if (srcNode != nullptr && eventMap_.find(transitionReason) != eventMap_.end()) {
        WLOGFI("current window:%{public}u state: %{public}u transitionReason:%{public}u", srcNode->GetWindowId(),
            static_cast<uint32_t>(srcNode->stateMachine_.GetCurrentState()), static_cast<uint32_t>(transitionReason));
        if (srcNode->stateMachine_.IsWindowNodeHiddenOrHiding()) {
            WLOGFE("srcNode is hiding or hidden id: %{public}u!", srcNode->GetWindowId());
            return TransitionEvent::UNKNOWN;
        }
        return eventMap_[transitionReason];
    }
    WLOGFI("Ability Transition");
    if (dstNode == nullptr) {
        if (dstInfo->GetAbilityToken() == nullptr) {
            WLOGFE("target window abilityToken is null");
        }
        return TransitionEvent::UNKNOWN;
    } else {
        WLOGFI("current window:%{public}u state: %{public}u", dstNode->GetWindowId(),
            static_cast<uint32_t>(dstNode->stateMachine_.GetCurrentState()));
        if (WindowHelper::IsMainWindow(dstInfo->GetWindowType())) {
            if (dstNode->stateMachine_.IsWindowNodeShownOrShowing()) {
                WLOGFE("dstNode is showing or shown id: %{public}d state:%{public}u!",
                    dstNode->GetWindowId(), static_cast<uint32_t>(dstNode->stateMachine_.GetCurrentState()));
                return TransitionEvent::UNKNOWN;
            }
            return TransitionEvent::APP_TRANSITION;
        } else if (dstInfo->GetWindowType() == WindowType::WINDOW_TYPE_DESKTOP) {
            return TransitionEvent::HOME;
        }
    }
    return TransitionEvent::UNKNOWN;
}

sptr<RSWindowAnimationFinishedCallback> RemoteAnimation::GetTransitionFinishedCallback(
    const sptr<WindowNode>& srcNode, const sptr<WindowNode>& dstNode)
{
    wptr<WindowNode> weak = dstNode;
    wptr<WindowNode> weakSrc = srcNode;
    auto callback = [weakSrc, weak]() {
        WLOGFI("RSWindowAnimation: on finish transition with minimize pre fullscreen!");
        auto weakNode = weak.promote();
        if (weakNode == nullptr) {
            WLOGFE("dst windowNode is nullptr!");
            return;
        }
        FinishAsyncTraceArgs(HITRACE_TAG_WINDOW_MANAGER, static_cast<int32_t>(TraceTaskId::REMOTE_ANIMATION),
            "wms:async:ShowRemoteAnimation");
        if (!weakNode->stateMachine_.IsWindowNodeShownOrShowing()) {
            WLOGFI("node:%{public}u is not play show animation with state:%{public}u!", weakNode->GetWindowId(),
                static_cast<uint32_t>(weakNode->stateMachine_.GetCurrentState()));
            return;
        }
        MinimizeApp::ExecuteMinimizeAll(); // minimize execute in show animation
        RSAnimationTimingProtocol timingProtocol(200); // animation time
        auto rsTransAdapter = std::make_shared<RSTransactionAdapter>(weakNode->leashWinSurfaceNode_);
        RSNode::Animate(rsTransAdapter->GetRSUIContext(), timingProtocol, RSAnimationTimingCurve::EASE_OUT,
            [weakNode, rsTransAdapter]() {
                auto winRect = weakNode->GetWindowRect();
                WLOGFD("name:%{public}s id:%{public}u winRect:[x:%{public}d, y:%{public}d, w:%{public}d, h:%{public}d]",
                    weakNode->GetWindowName().c_str(), weakNode->GetWindowId(),
                    winRect.posX_, winRect.posY_, winRect.width_, winRect.height_);
                if (!weakNode->leashWinSurfaceNode_) {
                    return;
                }
                weakNode->leashWinSurfaceNode_->SetBounds(
                    winRect.posX_, winRect.posY_, winRect.width_, winRect.height_);
                rsTransAdapter->FlushImplicitTransaction();
                weakNode->stateMachine_.TransitionTo(WindowNodeState::SHOW_ANIMATION_DONE);
        });
    };
    return CreateAnimationFinishedCallback(callback, dstNode);
}

WMError RemoteAnimation::NotifyAnimationStartApp(sptr<WindowTransitionInfo> srcInfo,
    const sptr<WindowNode>& srcNode, const sptr<WindowNode>& dstNode,
    sptr<RSWindowAnimationTarget>& dstTarget, sptr<RSWindowAnimationFinishedCallback>& finishedCallback)
{
    if (animationFirst_) {
        // From Recent also need to minimize window
        MinimizeApp::ExecuteMinimizeAll();
    }
    // start app from launcher
    if (srcNode != nullptr && srcNode->GetWindowType() == WindowType::WINDOW_TYPE_DESKTOP) {
        WLOGFI("RSWindowAnimation: start app id:%{public}u from launcher!", dstNode->GetWindowId());
        windowAnimationController_->OnStartApp(StartingAppType::FROM_LAUNCHER, dstTarget, finishedCallback);
        return WMError::WM_OK;
    }
    // start app from recent
    if (srcInfo != nullptr && srcInfo->GetIsRecent()) {
        WLOGFI("RSWindowAnimation: start app id:%{public}u from recent!", dstNode->GetWindowId());
        windowAnimationController_->OnStartApp(StartingAppType::FROM_RECENT, dstTarget, finishedCallback);
        return WMError::WM_OK;
    }
    // start app from other
    WLOGFI("RSWindowAnimation: start app id:%{public}u from other!", dstNode->GetWindowId());
    windowAnimationController_->OnStartApp(StartingAppType::FROM_OTHER, dstTarget, finishedCallback);
    return WMError::WM_OK;
}

void RemoteAnimation::GetExpectRect(const sptr<WindowNode>& dstNode, const sptr<RSWindowAnimationTarget>& dstTarget)
{
    // when exit immersive, startingWindow (0,0,w,h), but app need avoid
    bool needAvoid = (dstNode->GetWindowFlags() & static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_NEED_AVOID));
    auto winRoot = windowRoot_.promote();
    if (needAvoid && winRoot) {
        auto avoidRect = winRoot->GetDisplayRectWithoutSystemBarAreas(dstNode);
        if (WindowHelper::IsEmptyRect(avoidRect)) {
            return;
        }
        WLOGFI("name:%{public}s id:%{public}u avoidRect:[x:%{public}d, y:%{public}d, w:%{public}d, h:%{public}d]",
            dstNode->GetWindowName().c_str(), dstNode->GetWindowId(),
            avoidRect.posX_, avoidRect.posY_, avoidRect.width_, avoidRect.height_);
        if (WindowHelper::IsMainFullScreenWindow(dstNode->GetWindowType(), dstNode->GetWindowMode())) {
            auto boundsRect = RectF(avoidRect.posX_, avoidRect.posY_, avoidRect.width_, avoidRect.height_);
            auto displayInfo = DisplayGroupInfo::GetInstance().GetDisplayInfo(dstNode->GetDisplayId());
            if (displayInfo && WmsUtils::IsExpectedRotatableWindow(dstNode->GetRequestedOrientation(),
                displayInfo->GetDisplayOrientation(), dstNode->GetWindowFlags())) {
                WLOGFD("[FixOrientation] window %{public}u expected rotatable, pre-cal bounds", dstNode->GetWindowId());
                boundsRect = RectF(avoidRect.posX_, avoidRect.posY_, avoidRect.height_, avoidRect.width_);
            }
            dstTarget->windowBounds_.rect_ = boundsRect;
            if (dstNode->leashWinSurfaceNode_) {
                dstNode->leashWinSurfaceNode_->SetBounds(avoidRect.posX_, avoidRect.posY_,
                    avoidRect.width_, avoidRect.height_);
            }
        }
    }
}

WMError RemoteAnimation::NotifyAnimationTransition(sptr<WindowTransitionInfo> srcInfo,
    sptr<WindowTransitionInfo> dstInfo, const sptr<WindowNode>& srcNode,
    const sptr<WindowNode>& dstNode)
{
    if (!dstNode) {
        return WMError::WM_ERROR_NULLPTR;
    }
    WLOGFI("RSWindowAnimation: notify animation transition with dst currId:%{public}u!", dstNode->GetWindowId());
    bool needMinimizeSrcNode = MinimizeApp::IsNodeNeedMinimizeWithReason(srcNode, MinimizeReason::OTHER_WINDOW);
    auto finishedCallback = CreateShowAnimationFinishedCallback(srcNode, dstNode, needMinimizeSrcNode);
    if (finishedCallback == nullptr) {
        return WMError::WM_ERROR_NO_MEM;
    }
    auto dstTarget = CreateWindowAnimationTarget(dstInfo, dstNode);
    if (dstTarget == nullptr) {
        WLOGFE("RSWindowAnimation: Failed to create dst target!");
        finishedCallback->OnAnimationFinished();
        return WMError::WM_ERROR_NO_MEM;
    }

    std::unordered_map<std::string, std::string> payload;
    if (srcNode) {
        payload["srcPid"] = std::to_string(srcNode->GetCallingPid());
    }
    ResSchedReport::GetInstance().ResSchedDataReport(
        Rosen::RES_TYPE_SHOW_REMOTE_ANIMATION, Rosen::REMOTE_ANIMATION_BEGIN, payload);
    // when exit immersive, startingWindow (0,0,w,h), but app need avoid
    GetExpectRect(dstNode, dstTarget);
    dstNode->isPlayAnimationShow_ = true;
    // Transition to next state and update task count will success when enable animationFirst_
    dstNode->stateMachine_.TransitionTo(WindowNodeState::SHOW_ANIMATION_PLAYING);
    dstNode->stateMachine_.UpdateAnimationTaskCount(true);
    // from app to app
    if (needMinimizeSrcNode && srcNode != nullptr) {
        auto srcTarget = CreateWindowAnimationTarget(srcInfo, srcNode);
        // to avoid normal animation
        srcNode->isPlayAnimationHide_ = true;
        srcNode->stateMachine_.TransitionTo(WindowNodeState::HIDE_ANIMATION_PLAYING);
        srcNode->stateMachine_.UpdateAnimationTaskCount(true);
        auto winController = windowController_.promote();
        if (winController) {
            winController->RemoveWindowNode(srcNode->GetWindowId(), true);
        }
        if (animationFirst_) {
            // Notify minimize before animation when animationFirst is enable.
            // Or notify minimize in animation finished callback.
            MinimizeApp::ExecuteMinimizeAll();
        }
        WLOGFI("RSWindowAnimation: app transition from id:%{public}u to id:%{public}u!",
            srcNode->GetWindowId(), dstNode->GetWindowId());
        windowAnimationController_->OnAppTransition(srcTarget, dstTarget, finishedCallback);
        return WMError::WM_OK;
    }
    return NotifyAnimationStartApp(srcInfo, srcNode, dstNode, dstTarget, finishedCallback);
}

WMError RemoteAnimation::NotifyAnimationMinimize(sptr<WindowTransitionInfo> srcInfo, const sptr<WindowNode>& srcNode)
{
    sptr<RSWindowAnimationTarget> srcTarget = CreateWindowAnimationTarget(srcInfo, srcNode);
    if (srcTarget == nullptr) {
        return WMError::WM_ERROR_NO_MEM;
    }
    WLOGFI("RSWindowAnimation: notify animation minimize Id:%{public}u!", srcNode->GetWindowId());
    srcNode->isPlayAnimationHide_ = true;
    srcNode->stateMachine_.TransitionTo(WindowNodeState::HIDE_ANIMATION_PLAYING);
    auto winController = windowController_.promote();
    if (winController) {
        winController->RemoveWindowNode(srcNode->GetWindowId(), true);
    }
    sptr<RSWindowAnimationFinishedCallback> finishedCallback = CreateHideAnimationFinishedCallback(
        srcNode, TransitionEvent::MINIMIZE);
    if (finishedCallback == nullptr) {
        WLOGFE("New RSIWindowAnimationFinishedCallback failed");
        return WMError::WM_ERROR_NO_MEM;
    }

    srcNode->stateMachine_.TransitionTo(WindowNodeState::HIDE_ANIMATION_PLAYING);
    srcNode->stateMachine_.UpdateAnimationTaskCount(true);
    windowAnimationController_->OnMinimizeWindow(srcTarget, finishedCallback);
    return WMError::WM_OK;
}

WMError RemoteAnimation::NotifyAnimationClose(sptr<WindowTransitionInfo> srcInfo, const sptr<WindowNode>& srcNode,
    TransitionEvent event)
{
    sptr<RSWindowAnimationTarget> srcTarget = CreateWindowAnimationTarget(srcInfo, srcNode);
    if (srcTarget == nullptr) {
        return WMError::WM_ERROR_NO_MEM;
    }
    WLOGFI("RSWindowAnimation: notify animation close id:%{public}u!", srcNode->GetWindowId());
    srcNode->isPlayAnimationHide_ = true;
    auto winController = windowController_.promote();
    if (winController) {
        winController->RemoveWindowNode(srcNode->GetWindowId(), true);
    }
    sptr<RSWindowAnimationFinishedCallback> finishedCallback = CreateHideAnimationFinishedCallback(srcNode, event);
    if (finishedCallback == nullptr) {
        WLOGFE("New RSIWindowAnimationFinishedCallback failed");
        return WMError::WM_ERROR_NO_MEM;
    }
    srcNode->stateMachine_.TransitionTo(WindowNodeState::HIDE_ANIMATION_PLAYING);
    srcNode->stateMachine_.UpdateAnimationTaskCount(true);
    windowAnimationController_->OnCloseWindow(srcTarget, finishedCallback);
    return WMError::WM_OK;
}

void RemoteAnimation::NotifyAnimationAbilityDied(sptr<WindowTransitionInfo> info)
{
    if (info == nullptr) {
        WLOGFE("Window transition info is null!");
        return;
    }
    auto handler = wmsTaskHandler_.lock();
    if (handler == nullptr) {
        WLOGFE("wmsTaskHandler_ is nullptr");
        return;
    }
    // need post task when visit windowRoot node map
    auto task = [info]() {
        if (!CheckAnimationController()) {
            return;
        }
        WLOGFI("ability died bundleName:%{public}s, abilityName:%{public}s", info->GetBundleName().c_str(),
            info->GetAbilityName().c_str());
        sptr<RSWindowAnimationTarget> target = new(std::nothrow) RSWindowAnimationTarget();
        if (target == nullptr) {
            WLOGFE("target is nullptr");
            return;
        }
        target->bundleName_ = info->GetBundleName();
        target->abilityName_ = info->GetAbilityName();
        target->missionId_ = info->GetMissionId();
        target->windowId_ = INVALID_WINDOW_ID;
        auto func = []() { WLOGFI("NotifyAbilityDied finished!"); };
        auto finishCallback = CreateAnimationFinishedCallback(func, nullptr);
        windowAnimationController_->OnCloseWindow(target, finishCallback);
    };
    bool ret = handler->PostTask(task, "wms:NotifyAnimationAbilityDied",
        0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
    if (!ret) {
        WLOGFE("EventHandler PostTask Failed");
        task();
    }
}

WMError RemoteAnimation::NotifyAnimationBackTransition(sptr<WindowTransitionInfo> srcInfo,
    sptr<WindowTransitionInfo> dstInfo, const sptr<WindowNode>& srcNode,
    const sptr<WindowNode>& dstNode, const TransitionEvent event)
{
    if (!animationFirst_) {
        WLOGFE("not animation first!");
        return WMError::WM_ERROR_NO_REMOTE_ANIMATION;
    }
    if (!dstNode || !srcNode) {
        WLOGFE("dstNode or srcNode is nullptr, no need transition animation");
        return WMError::WM_ERROR_NULLPTR;
    }
    WLOGFI("RSWindowAnimation: app back transition from id:%{public}u to id:%{public}u!",
        srcNode->GetWindowId(), dstNode->GetWindowId());
    sptr<RSWindowAnimationTarget> srcTarget = CreateWindowAnimationTarget(srcInfo, srcNode);
    sptr<RSWindowAnimationTarget> dstTarget = CreateWindowAnimationTarget(dstInfo, dstNode);
    if (srcTarget == nullptr || dstTarget == nullptr) {
        return WMError::WM_ERROR_NO_MEM;
    }
    srcNode->isPlayAnimationHide_ = true;
    srcNode->stateMachine_.TransitionTo(WindowNodeState::HIDE_ANIMATION_PLAYING);
    srcNode->stateMachine_.UpdateAnimationTaskCount(true);
    auto winController = windowController_.promote();
    if (winController) {
        winController->RemoveWindowNode(srcNode->GetWindowId(), true);
        if (event == TransitionEvent::BACK_TRANSITION) {
            GetAndDrawSnapShot(srcNode);
        }
    }
    if (animationFirst_ && event == TransitionEvent::BACKGROUND_TRANSITION) {
        MinimizeApp::ExecuteMinimizeAll();
    }
    dstNode->isPlayAnimationShow_ = true;
    dstNode->stateMachine_.TransitionTo(WindowNodeState::SHOW_ANIMATION_PLAYING);
    dstNode->stateMachine_.UpdateAnimationTaskCount(true);
    wptr<WindowNode> srcNodeWptr = srcNode;
    wptr<WindowNode> dstNodeWptr = dstNode;
    auto func = [srcNodeWptr, dstNodeWptr]() {
        WLOGFI("RSWindowAnimation: animationFirst use state machine process AnimationBackTransition!");
        auto srcNodeSptr = srcNodeWptr.promote();
        auto dstNodeSptr = dstNodeWptr.promote();
        ProcessNodeStateTask(srcNodeSptr);
        ProcessNodeStateTask(dstNodeSptr);
        FinishAsyncTraceArgs(HITRACE_TAG_WINDOW_MANAGER, static_cast<int32_t>(TraceTaskId::REMOTE_ANIMATION),
            "wms:async:ShowRemoteAnimation");
    };
    sptr<RSWindowAnimationFinishedCallback> finishedCallback = CreateAnimationFinishedCallback(func, dstNode);
    if (finishedCallback == nullptr) {
        return WMError::WM_ERROR_NO_MEM;
    }
    windowAnimationController_->OnAppBackTransition(srcTarget, dstTarget, finishedCallback);
    return WMError::WM_OK;
}

void RemoteAnimation::GetAnimationTargetsForHome(std::vector<sptr<RSWindowAnimationTarget>>& animationTargets,
    std::vector<wptr<WindowNode>> needMinimizeAppNodes)
{
    for (auto& weakNode : needMinimizeAppNodes) {
        auto srcNode = weakNode.promote();
        sptr<WindowTransitionInfo> srcInfo = new(std::nothrow) WindowTransitionInfo();
        sptr<RSWindowAnimationTarget> srcTarget = CreateWindowAnimationTarget(srcInfo, srcNode);
        if (srcTarget == nullptr) {
            continue;
        }
        WLOGFI("notify animation by home, need minimize id%{public}u", srcNode->GetWindowId());
        if (!WindowHelper::IsMainWindow(srcNode->GetWindowType()) ||
            srcNode->stateMachine_.IsWindowNodeHiddenOrHiding()) {
            WLOGFE("srcNode is already hiding or hidden id: %{public}d!", srcNode->GetWindowId());
            continue;
        }
        StartAsyncTraceArgs(HITRACE_TAG_WINDOW_MANAGER, static_cast<int32_t>(TraceTaskId::REMOTE_ANIMATION_HOME),
            "wms:async:RemoteAnimationHome");
        srcNode->isPlayAnimationHide_ = true;
        srcNode->stateMachine_.TransitionTo(WindowNodeState::HIDE_ANIMATION_PLAYING);
        srcNode->stateMachine_.UpdateAnimationTaskCount(true);
        auto winController = windowController_.promote();
        if (winController) {
            winController->RemoveWindowNode(srcNode->GetWindowId(), true);
        }
        animationTargets.emplace_back(srcTarget);
    }
}

static void GetAnimationHomeFinishCallback(std::function<void(void)>& func,
    std::vector<wptr<WindowNode>> needMinimizeAppNodes)
{
    func = [needMinimizeAppNodes]() {
        WLOGFI("NotifyAnimationByHome in animation callback not animationFirst");
        for (auto& weakNode : needMinimizeAppNodes) {
            auto srcNode = weakNode.promote();
            if (srcNode == nullptr || !srcNode->stateMachine_.IsWindowNodeHiddenOrHiding()) {
                WLOGFE("windowNode is nullptr or is not play hide animation!");
                continue;
            }
            srcNode->stateMachine_.TransitionTo(WindowNodeState::HIDE_ANIMATION_DONE);
        }
        MinimizeApp::ExecuteMinimizeTargetReasons(MinimizeReason::MINIMIZE_ALL);
        FinishAsyncTraceArgs(HITRACE_TAG_WINDOW_MANAGER, static_cast<int32_t>(TraceTaskId::REMOTE_ANIMATION),
            "wms:async:ShowRemoteAnimation");
    };
}

WMError RemoteAnimation::NotifyAnimationByHome()
{
    if (!CheckAnimationController()) {
        return WMError::WM_ERROR_NO_REMOTE_ANIMATION;
    }
    auto needMinimizeAppNodes = MinimizeApp::GetNeedMinimizeAppNodesWithReason(MinimizeReason::MINIMIZE_ALL);
    WLOGFI("RSWindowAnimation: notify animation by home, need minimize size: %{public}u",
        static_cast<uint32_t>(needMinimizeAppNodes.size()));
    std::vector<sptr<RSWindowAnimationTarget>> animationTargets;
    GetAnimationTargetsForHome(animationTargets, needMinimizeAppNodes);
    std::function<void(void)> func;
    if (animationFirst_) {
        MinimizeApp::ExecuteMinimizeTargetReasons(MinimizeReason::MINIMIZE_ALL);
        func = [needMinimizeAppNodes]() {
            WLOGFI("NotifyAnimationByHome in animation callback in animationFirst with size:%{public}u",
                static_cast<uint32_t>(needMinimizeAppNodes.size()));
            for (auto& weakNode : needMinimizeAppNodes) {
                auto srcNode = weakNode.promote();
                ProcessNodeStateTask(srcNode);
                FinishAsyncTraceArgs(HITRACE_TAG_WINDOW_MANAGER,
                    static_cast<int32_t>(TraceTaskId::REMOTE_ANIMATION_HOME),
                    "wms:async:RemoteAnimationHome");
            }
        };
    } else {
        GetAnimationHomeFinishCallback(func, needMinimizeAppNodes);
    }
#ifdef SOC_PERF_ENABLE
    constexpr int32_t ACTION_TYPE_CPU_BOOST_CMDID = 10060;
    OHOS::SOCPERF::SocPerfClient::GetInstance().PerfRequestEx(ACTION_TYPE_CPU_BOOST_CMDID, true, "");
#endif
    sptr<RSWindowAnimationFinishedCallback> finishedCallback = CreateAnimationFinishedCallback(func, nullptr);
    if (finishedCallback == nullptr) {
        return WMError::WM_ERROR_NO_MEM;
    }
    // need use OnMinimizeWindows with controller
    windowAnimationController_->OnMinimizeAllWindow(animationTargets, finishedCallback);
    return WMError::WM_OK;
}

void RemoteAnimation::NotifyAnimationTargetsUpdate(std::vector<uint32_t>& fullScreenWinIds,
    std::vector<uint32_t>& floatWinIds)
{
    auto handler = wmsTaskHandler_.lock();
    if (handler == nullptr) {
        WLOGFE("wmsTaskHandler_ is nullptr");
        return;
    }
    // need post task when visit windowRoot node map
    auto task = [fullScreenWinIds, floatWinIds]() {
        if (!CheckAnimationController()) {
            return;
        }
        auto winRoot = windowRoot_.promote();
        if (winRoot == nullptr) {
            WLOGFE("window root is nullptr");
            return;
        }
        std::vector<sptr<RSWindowAnimationTarget>> floatAnimationTargets;
        std::vector<sptr<RSWindowAnimationTarget>> fullScreenAnimationTargets;
        for (auto& id : fullScreenWinIds) {
            auto fullScreenNode = winRoot->GetWindowNode(id);
            sptr<RSWindowAnimationTarget> fullScreenTarget = CreateWindowAnimationTarget(nullptr, fullScreenNode);
            if (fullScreenTarget != nullptr) {
                fullScreenAnimationTargets.emplace_back(fullScreenTarget);
            }
        }
        for (auto& id : floatWinIds) {
            auto floatNode = winRoot->GetWindowNode(id);
            sptr<RSWindowAnimationTarget> floatTarget = CreateWindowAnimationTarget(nullptr, floatNode);
            if (floatTarget != nullptr) {
                floatAnimationTargets.emplace_back(floatTarget);
            }
        }
        // delete when need all fullscreen targets
        sptr<RSWindowAnimationTarget> fullScreenAnimationTarget = nullptr;
        if (!fullScreenAnimationTargets.empty()) {
            fullScreenAnimationTarget = fullScreenAnimationTargets[0];
        }
        windowAnimationController_->OnWindowAnimationTargetsUpdate(fullScreenAnimationTarget,
            floatAnimationTargets);
    };
    bool ret = handler->PostTask(task, "wms:NotifyAnimationTargetsUpdate", 0,
        AppExecFwk::EventQueue::Priority::IMMEDIATE);
    if (!ret) {
        WLOGFE("EventHandler PostTask Failed");
        task();
    }
}

WMError RemoteAnimation::NotifyAnimationScreenUnlock(std::function<void(void)> callback, sptr<WindowNode> node)
{
    if (!CheckAnimationController()) {
        return WMError::WM_ERROR_NO_REMOTE_ANIMATION;
    }
    WLOGFI("NotifyAnimationScreenUnlock");
    sptr<RSWindowAnimationFinishedCallback> finishedCallback = CreateAnimationFinishedCallback(callback, node);
    if (finishedCallback == nullptr) {
        return WMError::WM_ERROR_NO_MEM;
    }

    windowAnimationController_->OnScreenUnlock(finishedCallback);
    return WMError::WM_OK;
}

WMError RemoteAnimation::GetWindowAnimationTargets(std::vector<uint32_t> missionIds,
    std::vector<sptr<RSWindowAnimationTarget>>& targets)
{
    auto winRoot = windowRoot_.promote();
    if (winRoot == nullptr) {
        WLOGFE("window root is nullptr");
        return WMError::WM_ERROR_NO_MEM;
    }
    for (uint32_t& missionId : missionIds) {
        sptr<WindowNode> windowNode = winRoot->GetWindowNodeByMissionId(missionId);
        auto target = CreateWindowAnimationTarget(nullptr, windowNode);
        if (target == nullptr) {
            continue;
        }
        targets.push_back(target);
    }
    return WMError::WM_OK;
}

sptr<RSWindowAnimationTarget> RemoteAnimation::CreateWindowAnimationTarget(sptr<WindowTransitionInfo> info,
    const sptr<WindowNode>& windowNode)
{
    if (windowNode == nullptr) {
        WLOGFW("Failed to create window animation target, window node is null!");
        return nullptr;
    }

    sptr<RSWindowAnimationTarget> windowAnimationTarget = new(std::nothrow) RSWindowAnimationTarget();
    if (windowAnimationTarget == nullptr) {
        WLOGFE("New RSWindowAnimationTarget failed");
        return nullptr;
    }

    if (WindowHelper::IsMainWindow(windowNode->GetWindowType())) { // only starting window has abilityInfo
        windowAnimationTarget->bundleName_ = windowNode->abilityInfo_.bundleName_;
        windowAnimationTarget->abilityName_ = windowNode->abilityInfo_.abilityName_;
    } else if (info) { // use for back, minimize, close
        windowAnimationTarget->bundleName_ = info->GetBundleName();
        windowAnimationTarget->abilityName_ = info->GetAbilityName();
    }

    windowAnimationTarget->missionId_ = windowNode->abilityInfo_.missionId_;
    windowAnimationTarget->windowId_ = windowNode->GetWindowId();
    windowAnimationTarget->displayId_ = windowNode->GetDisplayId();
    // some ability not has startingWindow，e.g. oobe
    if (WindowHelper::IsAppWindow(windowNode->GetWindowType()) && windowNode->leashWinSurfaceNode_) {
        windowAnimationTarget->surfaceNode_ = windowNode->leashWinSurfaceNode_;
    } else {
        windowAnimationTarget->surfaceNode_ = windowNode->surfaceNode_;
    }
    if (windowAnimationTarget->surfaceNode_ == nullptr) {
        WLOGFE("Window surface node is null id:%{public}u, type:%{public}u!",
            windowNode->GetWindowId(), windowNode->GetWindowType());
        return nullptr;
    }

    auto rect = windowNode->GetWindowRect();
    // 0, 1, 2, 3: convert bounds to RectF
    auto boundsRect = RectF(rect.posX_, rect.posY_, rect.width_, rect.height_);
    auto displayInfo = DisplayGroupInfo::GetInstance().GetDisplayInfo(windowNode->GetDisplayId());
    if (displayInfo && WmsUtils::IsExpectedRotatableWindow(windowNode->GetRequestedOrientation(),
        displayInfo->GetDisplayOrientation(), windowNode->GetWindowMode(), windowNode->GetWindowFlags())) {
        WLOGFD("[FixOrientation] the window %{public}u is expected rotatable, pre-calculate bounds, rect:"
            " [%{public}d, %{public}d, %{public}d, %{public}d]", windowNode->GetWindowId(), rect.posX_, rect.posY_,
            rect.height_, rect.width_);
        boundsRect = RectF(rect.posX_, rect.posY_, rect.height_, rect.width_);
    }
    auto& stagingProperties = windowAnimationTarget->surfaceNode_->GetStagingProperties();
    auto radius = stagingProperties.GetCornerRadius();
    windowAnimationTarget->windowBounds_ = RRect(boundsRect, radius);
    return windowAnimationTarget;
}

void RemoteAnimation::PostProcessShowCallback(const sptr<WindowNode>& node)
{
    if (node == nullptr) {
        WLOGFD("windowNode is nullptr!");
        return;
    }
    auto winRect = node->GetWindowRect();
    if (!node->leashWinSurfaceNode_) {
        WLOGFD("leashWinSurfaceNode_ is nullptr with id: %{public}u!", node->GetWindowId());
        return;
    }
    WLOGFD("name:%{public}s id:%{public}u winRect:[x:%{public}d, y:%{public}d, w:%{public}d, h:%{public}d]",
        node->GetWindowName().c_str(), node->GetWindowId(),
        winRect.posX_, winRect.posY_, winRect.width_, winRect.height_);
    node->leashWinSurfaceNode_->SetBounds(
        winRect.posX_, winRect.posY_, winRect.width_, winRect.height_);
    if (WindowHelper::IsRotatableWindow(node->GetWindowType(), node->GetWindowMode())) {
        auto displayId = node->GetDisplayId();
        auto requestOri = node->GetRequestedOrientation();
        WLOGFD("[FixOrientation] show animation finished, update display orientation");
        if (FIX_ORIENTATION_ENABLE) {
            DisplayManagerServiceInner::GetInstance().SetOrientationFromWindow(displayId, requestOri, false);
        } else {
            DisplayManagerServiceInner::GetInstance().SetOrientationFromWindow(displayId, requestOri, true);
        }
    }
    RSTransactionAdapter::FlushImplicitTransaction(node->leashWinSurfaceNode_);
}

void RemoteAnimation::ExecuteFinalStateTask(sptr<WindowNode>& node)
{
    StateTask destroyTask = nullptr;
    auto winRoot = windowRoot_.promote();
    if (winRoot == nullptr || node == nullptr) {
        WLOGFE("windowRoot or node is nullptr");
        return;
    }
    if (node->stateMachine_.IsWindowNodeHiddenOrHiding()) {
        WLOGFI("execute task removing from rs tree id:%{public}u!", node->GetWindowId());
        winRoot->UpdateRsTree(node->GetWindowId(), false);
    } else if (node->stateMachine_.IsWindowNodeShownOrShowing()) {
        WLOGFI("execute task layout after show animation id:%{public}u!", node->GetWindowId());
        winRoot->LayoutWhenAddWindowNode(node, true);
        WindowSystemEffect::SetWindowEffect(node, false); // no need to check animationPlaying in finishCallback
        auto winController = windowController_.promote();
        if (winController) {
            winController->FlushWindowInfo(node->GetWindowId());
        }
    } else {
        WLOGFD("current State:%{public}u invalid", static_cast<uint32_t>(node->stateMachine_.GetCurrentState()));
    }

    if (node->stateMachine_.GetDestroyTask(destroyTask)) {
        destroyTask();
    }
}

void RemoteAnimation::CallbackTimeOutProcess()
{
    auto winRoot = windowRoot_.promote();
    if (winRoot == nullptr) {
        WLOGFE("windowRoot is nullptr");
        return;
    }
    std::vector<wptr<WindowNode>> animationPlayingNodes;
    winRoot->GetAllAnimationPlayingNodes(animationPlayingNodes);
    WLOGFI("CallbackTimeOutProcess playingNodes:%{public}u", static_cast<uint32_t>(animationPlayingNodes.size()));
    for (auto& weakNode : animationPlayingNodes) {
        auto node = weakNode.promote();
        if (node == nullptr) {
            continue;
        }
        WLOGFI("callback timeout process windowId:%{public}u", node->GetWindowId());
        node->stateMachine_.ResetAnimationTaskCount(1);
        ProcessNodeStateTask(node);
    }
}

static void ReportWindowAnimationAbnormalInfo(sptr<WindowNode>& node)
{
    std::ostringstream oss;
    oss << "animation callback more than task: " << "window_name: " << node->GetWindowName() << ";";
    std::string info = oss.str();
    info += node->stateMachine_.GenStateMachineInfo();
    int32_t ret = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::WINDOW_MANAGER,
        "WINDOW_ANIMATION_ABNORMAL",
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT,
        "PID", node->GetCallingPid(),
        "UID", node->GetCallingUid(),
        "PACKAGE_NAME", node->abilityInfo_.abilityName_,
        "PROCESS_NAME", node->abilityInfo_.bundleName_,
        "MSG", info);
    if (ret != 0) {
        WLOGFE("Write HiSysEvent error, ret:%{public}d", ret);
    }
}

void RemoteAnimation::ProcessNodeStateTask(sptr<WindowNode>& node)
{
    // when callback come, node maybe destroyed
    if (node == nullptr) {
        WLOGFI("node is nullptr!");
        return;
    }
    int32_t taskCount = node->stateMachine_.GetAnimationCount();
    if (taskCount <= 0) { // no animation task but finishCallback come
        WLOGFE("ProcessNodeStateTask failed with windowId: %{public}u, name:%{public}s taskCount:%{public}d",
            node->GetWindowId(), node->GetWindowName().c_str(), taskCount);
        ReportWindowAnimationAbnormalInfo(node);
        return;
    }
    node->stateMachine_.UpdateAnimationTaskCount(false);
    taskCount = node->stateMachine_.GetAnimationCount();
    WLOGFI("ProcessNodeStateTask windowId: %{public}u, name:%{public}s state: %{public}u, taskCount:%{public}d",
        node->GetWindowId(), node->GetWindowName().c_str(),
        static_cast<uint32_t>(node->stateMachine_.GetCurrentState()), taskCount);
    if (taskCount > 0 || taskCount < 0) {
        WLOGFI("not last state task of window: %{public}d, %{public}d callback left not be executed!",
            node->GetWindowId(), taskCount);
        return;
    }
    ExecuteFinalStateTask(node);
    if (node->stateMachine_.IsWindowNodeShownOrShowing()) {
        // delete when immersive solution change
        PostProcessShowCallback(node);
        node->stateMachine_.TransitionTo(WindowNodeState::SHOW_ANIMATION_DONE);
    } else if (node->stateMachine_.IsWindowNodeHiddenOrHiding()) {
        node->stateMachine_.TransitionTo(WindowNodeState::HIDE_ANIMATION_DONE);
        auto winRoot = windowRoot_.promote();
        if (winRoot != nullptr) {
            winRoot->UpdateDisplayOrientationWhenHideWindow(node);
        }
    } else if (node->stateMachine_.GetCurrentState() == WindowNodeState::DESTROYED) {
        auto winRoot = windowRoot_.promote();
        if (winRoot != nullptr) {
            winRoot->UpdateDisplayOrientationWhenHideWindow(node);
        }
    }
}

sptr<RSWindowAnimationFinishedCallback> RemoteAnimation::CreateShowAnimationFinishedCallback(
    const sptr<WindowNode>& srcNode, const sptr<WindowNode>& dstNode, bool needMinimizeSrcNode)
{
    wptr<WindowNode> srcNodeWptr = srcNode;
    wptr<WindowNode> dstNodeWptr = dstNode;
    // need add timeout check
    std::function<void(void)> func;
    if (!animationFirst_) {
        WLOGFI("RSWindowAnimation: not animationFirst use default callback!");
        return GetTransitionFinishedCallback(srcNode, dstNode);
    } else {
        func = [srcNodeWptr, dstNodeWptr, needMinimizeSrcNode]() {
            WLOGFI("RSWindowAnimation: animationFirst use state machine process ShowAnimationFinishedCallback!");
            auto srcNodeSptr = srcNodeWptr.promote();
            auto dstNodeSptr = dstNodeWptr.promote();
            if (dstNodeSptr == nullptr) {
                WLOGFE("dstNode is nullptr!");
                return;
            }
            ProcessNodeStateTask(dstNodeSptr);
            // launcher not do this
            if (needMinimizeSrcNode) {
                ProcessNodeStateTask(srcNodeSptr);
            }
            WLOGFI("current window:%{public}u state: %{public}u", dstNodeSptr->GetWindowId(),
                static_cast<uint32_t>(dstNodeSptr->stateMachine_.GetCurrentState()));
            FinishAsyncTraceArgs(HITRACE_TAG_WINDOW_MANAGER, static_cast<int32_t>(TraceTaskId::REMOTE_ANIMATION),
                "wms:async:ShowRemoteAnimation");
            std::unordered_map<std::string, std::string> payload;
            if (srcNodeWptr != nullptr) {
                payload["srcPid"] = std::to_string(srcNodeWptr->GetCallingPid());
            }
            ResSchedReport::GetInstance().ResSchedDataReport(
                Rosen::RES_TYPE_SHOW_REMOTE_ANIMATION, Rosen::REMOTE_ANIMATION_END, payload);
        };
    }
    return CreateAnimationFinishedCallback(func, dstNode);
}

static void ProcessAbility(const sptr<WindowNode>& srcNode, TransitionEvent event)
{
    if (srcNode == nullptr) {
        return;
    }
    switch (event) {
        case TransitionEvent::CLOSE_BUTTON: {
            WLOGFI("close windowId: %{public}u, name:%{public}s",
                srcNode->GetWindowId(), srcNode->GetWindowName().c_str());
            WindowInnerManager::GetInstance().CloseAbility(srcNode);
            break;
        }
        case TransitionEvent::MINIMIZE: {
            WLOGFI("minimize windowId: %{public}u, name:%{public}s",
                srcNode->GetWindowId(), srcNode->GetWindowName().c_str());
            WindowInnerManager::GetInstance().MinimizeAbility(srcNode, true);
            break;
        }
        case TransitionEvent::CLOSE: // close by back
        default:
            break;
    }
}

void RemoteAnimation::NotifyAnimationUpdateWallpaper(sptr<WindowNode> node)
{
    if (!CheckAnimationController()) {
        return;
    }
    WLOGFI("NotifyAnimationUpdateWallpaper");
    sptr<RSWindowAnimationTarget> srcTarget = CreateWindowAnimationTarget(nullptr, node);
    // when wallpaper destroy, update with nullptr
    windowAnimationController_->OnWallpaperUpdate(srcTarget);
}

sptr<RSWindowAnimationFinishedCallback> RemoteAnimation::CreateHideAnimationFinishedCallback(
    const sptr<WindowNode>& srcNode, TransitionEvent event)
{
    wptr<WindowNode> srcNodeWptr = srcNode;
    // need add timeout check
    std::function<void(void)> func;
    if (!animationFirst_) {
        func = [srcNodeWptr, event]() {
            WLOGFI("RSWindowAnimation: not animationFirst use default callback!");
            auto weakNode = srcNodeWptr.promote();
            if (weakNode == nullptr || weakNode->abilityToken_ == nullptr) {
                WLOGFE("window node or ability token is nullptr");
                return;
            }
            if (!weakNode->stateMachine_.IsWindowNodeHiddenOrHiding()) {
                WLOGFE("window is not playing hide animation");
                return;
            }
            ProcessAbility(weakNode, event);
            weakNode->stateMachine_.TransitionTo(WindowNodeState::HIDE_ANIMATION_DONE);
            FinishAsyncTraceArgs(HITRACE_TAG_WINDOW_MANAGER, static_cast<int32_t>(TraceTaskId::REMOTE_ANIMATION),
                "wms:async:ShowRemoteAnimation");
        };
    } else {
        if (event != TransitionEvent::MINIMIZE) {
            GetAndDrawSnapShot(srcNode);
        }
        ProcessAbility(srcNode, event); // execute first when animationFirst
        func = [srcNodeWptr]() {
            WLOGFI("RSWindowAnimation: animationFirst use state machine process HideAnimationFinishedCallback!");
            auto srcNodeSptr = srcNodeWptr.promote();
            ProcessNodeStateTask(srcNodeSptr);
            FinishAsyncTraceArgs(HITRACE_TAG_WINDOW_MANAGER, static_cast<int32_t>(TraceTaskId::REMOTE_ANIMATION),
                "wms:async:ShowRemoteAnimation");
        };
    }
    return CreateAnimationFinishedCallback(func, srcNode);
}

static void ReportWindowAnimationCallbackTimeout(sptr<WindowNode>& node, const std::string& taskName)
{
    std::ostringstream oss;
    oss << "animation callback time out: " << "window_name: " << node->GetWindowName()
        << "callbackName: " << taskName << ";";
    std::string info = oss.str();
    int32_t ret = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::WINDOW_MANAGER,
        "ANIMATION_CALLBACK_TIMEOUT",
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT,
        "PID", node->GetCallingPid(),
        "UID", node->GetCallingUid(),
        "PACKAGE_NAME", node->abilityInfo_.abilityName_,
        "PROCESS_NAME", node->abilityInfo_.bundleName_,
        "MSG", info);
    if (ret != 0) {
        WLOGFE("Write HiSysEvent error, ret:%{public}d", ret);
    }
}

sptr<RSWindowAnimationFinishedCallback> RemoteAnimation::CreateAnimationFinishedCallback(
    const std::function<void(void)>& callback, sptr<WindowNode> windowNode)
{
    if (callback == nullptr) {
        WLOGFE("callback is null!");
        return nullptr;
    }
    auto currentId = allocationId_.fetch_add(1);
    std::string timeOutTaskName = ANIMATION_TIME_OUT_TASK + std::to_string(currentId);
    wptr<WindowNode> weakNode = windowNode;
    auto timeoutFunc = [callback, timeOutTaskName, weakNode]() {
        WLOGFE("callback %{public}s is time out!", timeOutTaskName.c_str());
        callback();
        auto node = weakNode.promote();
        if (node == nullptr) {
            WLOGFW("window node is null or is home event, not report abnormal info!");
            return;
        }
        ReportWindowAnimationCallbackTimeout(node, timeOutTaskName);
    };
    auto callbackTask = [callback, timeOutTaskName]() {
        auto handler = wmsTaskHandler_.lock();
        if (handler != nullptr) {
            handler->RemoveTask("wms:" + timeOutTaskName);
            WLOGFD("remove task %{public}s since animationCallback Come", timeOutTaskName.c_str());
            handler->PostTask(callback, "wms:CreateAnimationFinishedCallback",
                0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
        }
    };
    auto handlerSptr = wmsTaskHandler_.lock();
    if (handlerSptr != nullptr) {
        handlerSptr->PostTask(timeoutFunc, "wms:" + timeOutTaskName, ANIMATION_TIME_OUT_MILLISECONDS);
        WLOGFD("PostTask task %{public}s", timeOutTaskName.c_str());
    }
    sptr<RSWindowAnimationFinishedCallback> finishCallback = new(std::nothrow) RSWindowAnimationFinishedCallback(
        callbackTask);
    if (finishCallback == nullptr) {
        WLOGFE("New RSIWindowAnimationFinishedCallback failed");
        callbackTask();
        return nullptr;
    }
    return finishCallback;
}
} // Rosen
} // OHOS
