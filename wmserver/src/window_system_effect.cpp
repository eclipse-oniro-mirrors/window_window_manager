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

#include "window_system_effect.h"

#include <common/rs_common_def.h>
#include "color_parser.h"
#include "display_group_info.h"
#include "remote_animation.h"
#include "window_helper.h"
#include "window_inner_manager.h"
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "WindowSystemEffect"};
}

AppWindowEffectConfig WindowSystemEffect::windowSystemEffectConfig_;
wptr<WindowRoot> WindowSystemEffect::windowRoot_;

void WindowSystemEffect::SetWindowSystemEffectConfig(AppWindowEffectConfig config)
{
    windowSystemEffectConfig_ = config;
}

void WindowSystemEffect::SetWindowRoot(const sptr<WindowRoot>& windowRoot)
{
    windowRoot_ = windowRoot;
}

WMError WindowSystemEffect::SetCornerRadius(const sptr<WindowNode>& node, bool needCheckAnimation)
{
    auto winRoot = windowRoot_.promote();
    if (winRoot == nullptr || node == nullptr) {
        WLOGFE("window root is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    // if change mode during animation, not set radius until animationFinish
    if (needCheckAnimation && RemoteAnimation::IsRemoteAnimationEnabledAndFirst(node->GetDisplayId()) &&
        node->stateMachine_.IsShowAnimationPlaying()) {
        WLOGFW("not set radius during animation");
        return WMError::WM_DO_NOTHING;
    }

    if (!IsAppMainOrSubOrFloatingWindow(node)) {
        return WMError::WM_DO_NOTHING;
    }
    auto vpr = DisplayGroupInfo::GetInstance().GetDisplayVirtualPixelRatio(node->GetDisplayId());
    auto fullscreenRadius = windowSystemEffectConfig_.fullScreenCornerRadius_ * vpr;
    auto splitRadius = windowSystemEffectConfig_.splitCornerRadius_ * vpr;
    auto floatRadius = windowSystemEffectConfig_.floatCornerRadius_ * vpr;

    WLOGFD("[WEffect] [id:%{public}d] mode: %{public}u, vpr: %{public}f, [%{public}f, %{public}f, %{public}f]",
        node->GetWindowId(), node->GetWindowMode(), vpr, fullscreenRadius, splitRadius, floatRadius);
    if (MathHelper::NearZero(fullscreenRadius) && MathHelper::NearZero(splitRadius) &&
        MathHelper::NearZero(floatRadius)) {
        return WMError::WM_DO_NOTHING;
    }
    auto surfaceNode = node->leashWinSurfaceNode_ != nullptr ? node->leashWinSurfaceNode_ : node->surfaceNode_;
    if (surfaceNode == nullptr) {
        WLOGFE("window surfaceNode is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    if (WindowHelper::IsFullScreenWindow(node->GetWindowMode())) {
        surfaceNode->SetCornerRadius(fullscreenRadius);
    } else if (WindowHelper::IsSplitWindowMode(node->GetWindowMode())) {
        surfaceNode->SetCornerRadius(splitRadius);
    } else if (WindowHelper::IsFloatingWindow(node->GetWindowMode())) {
        surfaceNode->SetCornerRadius(floatRadius);
    }
    return WMError::WM_OK;
}

bool WindowSystemEffect::IsAppMainOrSubOrFloatingWindow(const sptr<WindowNode>& node)
{
    if (WindowHelper::IsAppWindow(node->GetWindowType())) {
        return true;
    }
    auto winRoot = windowRoot_.promote();
    if (winRoot == nullptr) {
        return false;
    }
    // Type float has main window
    if (WindowHelper::IsAppFloatingWindow(node->GetWindowType()) &&
        winRoot->FindMainWindowWithToken(node->abilityToken_)) {
        return true;
    }
    WLOGFD("not appWindow or app floating window, id: %{public}u!", node->GetWindowId());
    return false;
}

WMError WindowSystemEffect::SetWindowShadow(const sptr<WindowNode>& node)
{
    auto winRoot = windowRoot_.promote();
    if (winRoot == nullptr || node == nullptr) {
        return WMError::WM_ERROR_NULLPTR;
    }

    if (!IsAppMainOrSubOrFloatingWindow(node)) {
        return WMError::WM_DO_NOTHING;
    }

    if (MathHelper::NearZero(windowSystemEffectConfig_.focusedShadow_.elevation_) &&
        MathHelper::NearZero(windowSystemEffectConfig_.unfocusedShadow_.elevation_) &&
        MathHelper::NearZero(windowSystemEffectConfig_.focusedShadow_.radius_) &&
        MathHelper::NearZero(windowSystemEffectConfig_.unfocusedShadow_.radius_)) {
        WLOGFD("shadow elevation and radius are both 0.0, id: %{public}u", node->GetWindowId());
        return WMError::WM_DO_NOTHING;
    }

    auto surfaceNode = node->leashWinSurfaceNode_ != nullptr ? node->leashWinSurfaceNode_ : node->surfaceNode_;
    if (surfaceNode == nullptr) {
        WLOGFE("window surfaceNode is null");
        return WMError::WM_ERROR_NULLPTR;
    }

    auto& shadow = node->isFocused_ ? windowSystemEffectConfig_.focusedShadow_ :
        windowSystemEffectConfig_.unfocusedShadow_;

    // when float mode change to fullscreen/split mode
    if (!WindowHelper::IsFloatingWindow(node->GetWindowMode())) {
        if (MathHelper::GreatNotEqual(shadow.elevation_, 0.f)) {
            surfaceNode->SetShadowElevation(0.f);
        } else {
            surfaceNode->SetShadowRadius(-1.0f);
        }
        surfaceNode->SetShadowAlpha(0.f);
        WLOGFD("[WEffect]close shadow id: %{public}u", node->GetWindowId());
        return WMError::WM_OK;
    }

    uint32_t colorValue;
    if (!ColorParser::Parse(shadow.color_, colorValue)) {
        WLOGFE("[WEffect]invalid color string: %{public}s", shadow.color_.c_str());
        return WMError::WM_ERROR_INVALID_PARAM;
    }

    WLOGFI("[WEffect]id: %{public}u focused: %{public}d elevation: %{public}f",
        node->GetWindowId(), static_cast<int32_t>(node->isFocused_), shadow.elevation_);
    WLOGFI("[WEffect]color: %{public}s offsetX: %{public}f offsetY: %{public}f alpha: %{public}f radius: %{public}f",
        shadow.color_.c_str(), shadow.offsetX_, shadow.offsetY_, shadow.alpha_, shadow.radius_);
    auto vpr = DisplayGroupInfo::GetInstance().GetDisplayVirtualPixelRatio(node->GetDisplayId());
    if (MathHelper::GreatNotEqual(shadow.elevation_, 0.f)) {
        surfaceNode->SetShadowElevation(shadow.elevation_ * vpr);
    } else {
        surfaceNode->SetShadowRadius(MathHelper::(shadow.radius_, 0.0) ?
            -1.0f : ConvertRadiusToSigma(shadow.radius_ * vpr));
    }
    surfaceNode->SetShadowColor(colorValue);
    surfaceNode->SetShadowOffsetX(shadow.offsetX_ * vpr);
    surfaceNode->SetShadowOffsetY(shadow.offsetY_ * vpr);
    surfaceNode->SetShadowAlpha(shadow.alpha_);
    return WMError::WM_OK;
}

WMError WindowSystemEffect::SetWindowEffect(const sptr<WindowNode>& node, bool needCheckAnimation)
{
    auto winRoot = windowRoot_.promote();
    if (node == nullptr) {
        WLOGFE("window node is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    SetCornerRadius(node, needCheckAnimation);
    SetWindowShadow(node);
    return WMError::WM_OK;
}
} // Rosen
} // OHOS
