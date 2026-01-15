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

#include "common/include/window_session_property.h"
#include "string_util.h"
#include "window_manager_hilog.h"
#include "wm_common.h"
#include "window_helper.h"

namespace OHOS {
namespace Rosen {
namespace {
constexpr uint32_t TOUCH_HOT_AREA_MAX_NUM = 50;
constexpr uint32_t TRANSITION_ANIMATION_MAP_SIZE_MAX_NUM = 100;
}

const std::map<uint64_t, HandlWritePropertyFunc> WindowSessionProperty::writeFuncMap_ {
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_TURN_SCREEN_ON),
        &WindowSessionProperty::WriteActionUpdateTurnScreenOn),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_KEEP_SCREEN_ON),
        &WindowSessionProperty::WriteActionUpdateKeepScreenOn),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_VIEW_KEEP_SCREEN_ON),
        &WindowSessionProperty::WriteActionUpdateViewKeepScreenOn),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_FOCUSABLE),
        &WindowSessionProperty::WriteActionUpdateFocusable),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_TOUCHABLE),
        &WindowSessionProperty::WriteActionUpdateTouchable),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_SET_BRIGHTNESS),
        &WindowSessionProperty::WriteActionUpdateSetBrightness),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_ORIENTATION),
        &WindowSessionProperty::WriteActionUpdateOrientation),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_PRIVACY_MODE),
        &WindowSessionProperty::WriteActionUpdatePrivacyMode),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_SYSTEM_PRIVACY_MODE),
        &WindowSessionProperty::WriteActionUpdatePrivacyMode),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_SNAPSHOT_SKIP),
        &WindowSessionProperty::WriteActionUpdateSnapshotSkip),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_MAXIMIZE_STATE),
        &WindowSessionProperty::WriteActionUpdateMaximizeState),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_OTHER_PROPS),
        &WindowSessionProperty::WriteActionUpdateSystemBar),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_STATUS_PROPS),
        &WindowSessionProperty::WriteActionUpdateSystemBar),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_NAVIGATION_PROPS),
        &WindowSessionProperty::WriteActionUpdateSystemBar),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_NAVIGATION_INDICATOR_PROPS),
        &WindowSessionProperty::WriteActionUpdateSystemBar),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_FLAGS),
        &WindowSessionProperty::WriteActionUpdateFlags),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_MODE),
        &WindowSessionProperty::WriteActionUpdateMode),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_ANIMATION_FLAG),
        &WindowSessionProperty::WriteActionUpdateAnimationFlag),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_TOUCH_HOT_AREA),
        &WindowSessionProperty::WriteActionUpdateTouchHotArea),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_KEYBOARD_TOUCH_HOT_AREA),
        &WindowSessionProperty::WriteActionUpdateKeyboardTouchHotArea),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_DECOR_ENABLE),
        &WindowSessionProperty::WriteActionUpdateDecorEnable),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_WINDOW_LIMITS),
        &WindowSessionProperty::WriteActionUpdateWindowLimits),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_DRAGENABLED),
        &WindowSessionProperty::WriteActionUpdateDragenabled),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_RAISEENABLED),
        &WindowSessionProperty::WriteActionUpdateRaiseenabled),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_HIDE_NON_SYSTEM_FLOATING_WINDOWS),
        &WindowSessionProperty::WriteActionUpdateHideNonSystemFloatingWindows),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_TEXTFIELD_AVOID_INFO),
        &WindowSessionProperty::WriteActionUpdateTextfieldAvoidInfo),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_WINDOW_MASK),
        &WindowSessionProperty::WriteActionUpdateWindowMask),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_TOPMOST),
        &WindowSessionProperty::WriteActionUpdateTopmost),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_SUB_WINDOW_Z_LEVEL),
        &WindowSessionProperty::WriteActionUpdateSubWindowZLevel),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_MODE_SUPPORT_INFO),
        &WindowSessionProperty::WriteActionUpdateWindowModeSupportType),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_MAIN_WINDOW_TOPMOST),
        &WindowSessionProperty::WriteActionUpdateMainWindowTopmost),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_AVOID_AREA_OPTION),
        &WindowSessionProperty::WriteActionUpdateAvoidAreaOption),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_BACKGROUND_ALPHA),
        &WindowSessionProperty::WriteActionUpdateBackgroundAlpha),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_EXCLUSIVE_HIGHLIGHTED),
        &WindowSessionProperty::WriteActionUpdateExclusivelyHighlighted),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_FOLLOW_SCREEN_CHANGE),
        &WindowSessionProperty::WriteActionUpdateFollowScreenChange),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_WINDOW_SHADOW_ENABLED),
        &WindowSessionProperty::WriteActionUpdateWindowShadowEnabled),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_ASPECT_RATIO),
        &WindowSessionProperty::WriteActionUpdateAspectRatio),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_ROTATION_LOCK_CHANGE),
        &WindowSessionProperty::WriteActionUpdateRotationLockChange),
};

const std::map<uint64_t, HandlReadPropertyFunc> WindowSessionProperty::readFuncMap_ {
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_TURN_SCREEN_ON),
        &WindowSessionProperty::ReadActionUpdateTurnScreenOn),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_KEEP_SCREEN_ON),
        &WindowSessionProperty::ReadActionUpdateKeepScreenOn),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_VIEW_KEEP_SCREEN_ON),
        &WindowSessionProperty::ReadActionUpdateViewKeepScreenOn),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_FOCUSABLE),
        &WindowSessionProperty::ReadActionUpdateFocusable),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_TOUCHABLE),
        &WindowSessionProperty::ReadActionUpdateTouchable),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_SET_BRIGHTNESS),
        &WindowSessionProperty::ReadActionUpdateSetBrightness),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_ORIENTATION),
        &WindowSessionProperty::ReadActionUpdateOrientation),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_PRIVACY_MODE),
        &WindowSessionProperty::ReadActionUpdatePrivacyMode),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_SYSTEM_PRIVACY_MODE),
        &WindowSessionProperty::ReadActionUpdatePrivacyMode),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_SNAPSHOT_SKIP),
        &WindowSessionProperty::ReadActionUpdateSnapshotSkip),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_MAXIMIZE_STATE),
        &WindowSessionProperty::ReadActionUpdateMaximizeState),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_OTHER_PROPS),
        &WindowSessionProperty::ReadActionUpdateSystemBar),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_STATUS_PROPS),
        &WindowSessionProperty::ReadActionUpdateSystemBar),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_NAVIGATION_PROPS),
        &WindowSessionProperty::ReadActionUpdateSystemBar),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_NAVIGATION_INDICATOR_PROPS),
        &WindowSessionProperty::ReadActionUpdateSystemBar),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_FLAGS),
        &WindowSessionProperty::ReadActionUpdateFlags),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_MODE),
        &WindowSessionProperty::ReadActionUpdateMode),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_ANIMATION_FLAG),
        &WindowSessionProperty::ReadActionUpdateAnimationFlag),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_TOUCH_HOT_AREA),
        &WindowSessionProperty::ReadActionUpdateTouchHotArea),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_KEYBOARD_TOUCH_HOT_AREA),
        &WindowSessionProperty::ReadActionUpdateKeyboardTouchHotArea),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_DECOR_ENABLE),
        &WindowSessionProperty::ReadActionUpdateDecorEnable),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_WINDOW_LIMITS),
        &WindowSessionProperty::ReadActionUpdateWindowLimits),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_DRAGENABLED),
        &WindowSessionProperty::ReadActionUpdateDragenabled),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_RAISEENABLED),
        &WindowSessionProperty::ReadActionUpdateRaiseenabled),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_HIDE_NON_SYSTEM_FLOATING_WINDOWS),
        &WindowSessionProperty::ReadActionUpdateHideNonSystemFloatingWindows),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_TEXTFIELD_AVOID_INFO),
        &WindowSessionProperty::ReadActionUpdateTextfieldAvoidInfo),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_WINDOW_MASK),
        &WindowSessionProperty::ReadActionUpdateWindowMask),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_TOPMOST),
        &WindowSessionProperty::ReadActionUpdateTopmost),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_SUB_WINDOW_Z_LEVEL),
        &WindowSessionProperty::ReadActionUpdateSubWindowZLevel),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_MODE_SUPPORT_INFO),
        &WindowSessionProperty::ReadActionUpdateWindowModeSupportType),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_MAIN_WINDOW_TOPMOST),
        &WindowSessionProperty::ReadActionUpdateMainWindowTopmost),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_AVOID_AREA_OPTION),
        &WindowSessionProperty::ReadActionUpdateAvoidAreaOption),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_BACKGROUND_ALPHA),
        &WindowSessionProperty::ReadActionUpdateBackgroundAlpha),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_EXCLUSIVE_HIGHLIGHTED),
        &WindowSessionProperty::ReadActionUpdateExclusivelyHighlighted),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_FOLLOW_SCREEN_CHANGE),
        &WindowSessionProperty::ReadActionUpdateFollowScreenChange),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_WINDOW_SHADOW_ENABLED),
        &WindowSessionProperty::ReadActionUpdateWindowShadowEnabled),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_ASPECT_RATIO),
        &WindowSessionProperty::ReadActionUpdateAspectRatio),
    std::make_pair(static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_ROTATION_LOCK_CHANGE),
        &WindowSessionProperty::ReadActionUpdateRotationLockChange),
};

WindowSessionProperty::WindowSessionProperty(const sptr<WindowSessionProperty>& property)
{
    CopyFrom(property);
}

void WindowSessionProperty::SetWindowName(const std::string& name)
{
    windowName_ = name;
}

void WindowSessionProperty::SetSessionInfo(const SessionInfo& info)
{
    sessionInfo_ = info;
}

void WindowSessionProperty::SetTransitionAnimationConfig(WindowTransitionType transitionType,
    const TransitionAnimation& animation)
{
    transitionAnimationConfig_[transitionType] = std::make_shared<TransitionAnimation>(animation);
}

void WindowSessionProperty::SetWindowRect(const struct Rect& rect)
{
    std::lock_guard<std::mutex> lock(windowRectMutex_);
    windowRect_ = rect;
}

void WindowSessionProperty::SetRequestRect(const Rect& requestRect)
{
    std::lock_guard<std::mutex> lock(requestRectMutex_);
    requestRect_ = requestRect;
}

void WindowSessionProperty::SetRectAnimationConfig(const RectAnimationConfig& rectAnimationConfig)
{
    std::lock_guard<std::mutex> lock(rectAnimationConfigMutex_);
    rectAnimationConfig_ = rectAnimationConfig;
}

void WindowSessionProperty::SetWindowType(WindowType type)
{
    type_ = type;
}

void WindowSessionProperty::SetFocusable(bool isFocusable)
{
    focusable_ = isFocusable;
}

void WindowSessionProperty::SetFocusableOnShow(bool isFocusableOnShow)
{
    focusableOnShow_ = isFocusableOnShow;
}

void WindowSessionProperty::SetTouchable(bool isTouchable)
{
    touchable_ = isTouchable;
}

void WindowSessionProperty::SetDragEnabled(bool dragEnabled)
{
    dragEnabled_ = dragEnabled;
}

void WindowSessionProperty::SetHideNonSystemFloatingWindows(bool hide)
{
    hideNonSystemFloatingWindows_ = hide;
}

void WindowSessionProperty::SetSkipSelfWhenShowOnVirtualScreen(bool isSkip)
{
    isSkipSelfWhenShowOnVirtualScreen_ = isSkip;
}

void WindowSessionProperty::SetSkipEventOnCastPlus(bool isSkip)
{
    isSkipEventOnCastPlus_ = isSkip;
}

void WindowSessionProperty::SetForceHide(bool hide)
{
    forceHide_ = hide;
}

void WindowSessionProperty::SetRaiseEnabled(bool raiseEnabled)
{
    raiseEnabled_ = raiseEnabled;
}

void WindowSessionProperty::SetRequestedOrientation(Orientation orientation, bool needAnimation)
{
    requestedOrientation_ = orientation;
    needRotateAnimation_ = needAnimation;
}

void WindowSessionProperty::SetDefaultRequestedOrientation(Orientation orientation)
{
    defaultRequestedOrientation_ = orientation;
}

void WindowSessionProperty::SetUserRequestedOrientation(Orientation orientation)
{
    userRequestedOrientation_ = orientation;
}

void WindowSessionProperty::SetIsSpecificSessionRequestOrientation(bool isSpecificSessionRequestOrientation)
{
    isSpecificSessionRequestOrientation_ = isSpecificSessionRequestOrientation;
}

void WindowSessionProperty::SetPrivacyMode(bool isPrivate)
{
    isPrivacyMode_ = isPrivate;
}

void WindowSessionProperty::SetSystemPrivacyMode(bool isSystemPrivate)
{
    isSystemPrivacyMode_ = isSystemPrivate;
}

void WindowSessionProperty::SetSnapshotSkip(bool isSkip)
{
    isSnapshotSkip_ = isSkip;
}

void WindowSessionProperty::SetBrightness(float brightness)
{
    brightness_ = brightness;
}

void WindowSessionProperty::SetSystemCalling(bool isSystemCalling)
{
    isSystemCalling_ = isSystemCalling;
}

void WindowSessionProperty::SetDisplayId(DisplayId displayId)
{
    displayId_ = displayId;
}

void WindowSessionProperty::SetIsFollowParentWindowDisplayId(bool enabled)
{
    isFollowParentWindowDisplayId_ = enabled;
}

void WindowSessionProperty::SetFloatingWindowAppType(bool isAppType)
{
    isFloatingWindowAppType_ = isAppType;
}

const std::string& WindowSessionProperty::GetWindowName() const
{
    return windowName_;
}

const SessionInfo& WindowSessionProperty::GetSessionInfo() const
{
    return sessionInfo_;
}

TransitionAnimationMapType WindowSessionProperty::GetTransitionAnimationConfig() const
{
    return transitionAnimationConfig_;
}

SessionInfo& WindowSessionProperty::EditSessionInfo()
{
    return sessionInfo_;
}

void WindowSessionProperty::SetGlobalDisplayRect(const Rect& globalDisplayRect)
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "globalDisplayRect=%{public}s", globalDisplayRect.ToString().c_str());
    std::lock_guard<std::mutex> lock(globalDisplayRectMutex_);
    globalDisplayRect_ = globalDisplayRect;
}

Rect WindowSessionProperty::GetGlobalDisplayRect() const
{
    std::lock_guard<std::mutex> lock(globalDisplayRectMutex_);
    return globalDisplayRect_;
}

Rect WindowSessionProperty::GetWindowRect() const
{
    std::lock_guard<std::mutex> lock(windowRectMutex_);
    return windowRect_;
}

Rect WindowSessionProperty::GetRequestRect() const
{
    std::lock_guard<std::mutex> lock(requestRectMutex_);
    return requestRect_;
}

RectAnimationConfig WindowSessionProperty::GetRectAnimationConfig() const
{
    std::lock_guard<std::mutex> lock(rectAnimationConfigMutex_);
    return rectAnimationConfig_;
}

WindowType WindowSessionProperty::GetWindowType() const
{
    return type_;
}

bool WindowSessionProperty::GetFocusable() const
{
    return focusable_;
}

bool WindowSessionProperty::GetFocusableOnShow() const
{
    return focusableOnShow_;
}

bool WindowSessionProperty::GetTouchable() const
{
    return touchable_;
}

bool WindowSessionProperty::GetDragEnabled() const
{
    return dragEnabled_;
}

bool WindowSessionProperty::GetHideNonSystemFloatingWindows() const
{
    return hideNonSystemFloatingWindows_;
}

bool WindowSessionProperty::GetSkipSelfWhenShowOnVirtualScreen() const
{
    return isSkipSelfWhenShowOnVirtualScreen_;
}

bool WindowSessionProperty::GetSkipEventOnCastPlus() const
{
    return isSkipEventOnCastPlus_;
}

bool WindowSessionProperty::GetForceHide() const
{
    return forceHide_;
}

bool WindowSessionProperty::GetRaiseEnabled() const
{
    return raiseEnabled_;
}

Orientation WindowSessionProperty::GetRequestedOrientation() const
{
    return requestedOrientation_;
}

bool WindowSessionProperty::GetRequestedAnimation() const
{
    return needRotateAnimation_;
}

Orientation WindowSessionProperty::GetDefaultRequestedOrientation() const
{
    return defaultRequestedOrientation_;
}

Orientation WindowSessionProperty::GetUserRequestedOrientation() const
{
    return userRequestedOrientation_;
}

bool WindowSessionProperty::GetIsSpecificSessionRequestOrientation() const
{
    return isSpecificSessionRequestOrientation_;
}

bool WindowSessionProperty::GetPrivacyMode() const
{
    return isPrivacyMode_;
}

bool WindowSessionProperty::GetSystemPrivacyMode() const
{
    return isSystemPrivacyMode_;
}

bool WindowSessionProperty::GetSnapshotSkip() const
{
    return isSnapshotSkip_;
}

float WindowSessionProperty::GetBrightness() const
{
    return brightness_;
}

bool WindowSessionProperty::GetSystemCalling() const
{
    return isSystemCalling_;
}

DisplayId WindowSessionProperty::GetDisplayId() const
{
    return displayId_;
}

bool WindowSessionProperty::IsFollowParentWindowDisplayId() const
{
    return isFollowParentWindowDisplayId_;
}

void WindowSessionProperty::SetParentId(int32_t parentId)
{
    parentId_ = parentId;
}

int32_t WindowSessionProperty::GetParentId() const
{
    return parentId_;
}

void WindowSessionProperty::SetWindowFlags(uint32_t flags)
{
    flags_ = flags;
}

void WindowSessionProperty::SetTopmost(bool topmost)
{
    topmost_ = topmost;
}

bool WindowSessionProperty::IsTopmost() const
{
    return topmost_;
}

void WindowSessionProperty::SetAvoidAreaOption(uint32_t avoidAreaOption)
{
    avoidAreaOption_ = avoidAreaOption;
}

uint32_t WindowSessionProperty::GetAvoidAreaOption() const
{
    return avoidAreaOption_;
}

void WindowSessionProperty::SetMainWindowTopmost(bool isTopmost)
{
    mainWindowTopmost_ = isTopmost;
}

bool WindowSessionProperty::IsMainWindowTopmost() const
{
    return mainWindowTopmost_;
}

void WindowSessionProperty::SetWindowDelayRaiseEnabled(bool isEnabled)
{
    isWindowDelayRaiseEnabled_ = isEnabled;
}

bool WindowSessionProperty::IsWindowDelayRaiseEnabled() const
{
    return isWindowDelayRaiseEnabled_;
}

void WindowSessionProperty::AddWindowFlag(WindowFlag flag)
{
    flags_ |= static_cast<uint32_t>(flag);
}

uint32_t WindowSessionProperty::GetWindowFlags() const
{
    return flags_;
}

void WindowSessionProperty::SetPersistentId(int32_t persistentId)
{
    persistentId_ = persistentId;
}

int32_t WindowSessionProperty::GetPersistentId() const
{
    return persistentId_;
}

void WindowSessionProperty::SetParentPersistentId(int32_t persistentId)
{
    parentPersistentId_ = persistentId;
}

int32_t WindowSessionProperty::GetParentPersistentId() const
{
    return parentPersistentId_;
}

void WindowSessionProperty::SetTurnScreenOn(bool turnScreenOn)
{
    turnScreenOn_ = turnScreenOn;
}

bool WindowSessionProperty::IsTurnScreenOn() const
{
    return turnScreenOn_;
}

void WindowSessionProperty::SetKeepScreenOn(bool keepScreenOn)
{
    keepScreenOn_ = keepScreenOn;
}

bool WindowSessionProperty::IsKeepScreenOn() const
{
    return keepScreenOn_;
}

void WindowSessionProperty::SetViewKeepScreenOn(bool keepScreenOn)
{
    viewKeepScreenOn_ = keepScreenOn;
}

bool WindowSessionProperty::IsViewKeepScreenOn() const
{
    return viewKeepScreenOn_;
}

void WindowSessionProperty::SetWindowShadowEnabled(bool isEnabled)
{
    windowShadowEnabled_ = isEnabled;
}

bool WindowSessionProperty::GetWindowShadowEnabled() const
{
    return windowShadowEnabled_;
}

void WindowSessionProperty::SetAccessTokenId(uint32_t accessTokenId)
{
    accessTokenId_ = accessTokenId;
}

uint32_t WindowSessionProperty::GetAccessTokenId() const
{
    return accessTokenId_;
}

void WindowSessionProperty::SetTokenState(bool hasToken)
{
    tokenState_ = hasToken;
}

bool WindowSessionProperty::GetTokenState() const
{
    return tokenState_;
}

MaximizeMode WindowSessionProperty::GetMaximizeMode() const
{
    return maximizeMode_;
}

void WindowSessionProperty::SetMaximizeMode(MaximizeMode mode)
{
    maximizeMode_ = mode;
}

void WindowSessionProperty::SetFollowScreenChange(bool isFollowScreenChange)
{
    isFollowScreenChange_ = isFollowScreenChange;
}

bool WindowSessionProperty::GetFollowScreenChange() const
{
    return isFollowScreenChange_;
}

void WindowSessionProperty::SetSystemBarProperty(WindowType type, const SystemBarProperty& property)
{
    if (type == WindowType::WINDOW_TYPE_STATUS_BAR ||
        type == WindowType::WINDOW_TYPE_NAVIGATION_BAR ||
        type == WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR) {
        sysBarPropMap_[type] = property;
    }
}

std::unordered_map<WindowType, SystemBarProperty> WindowSessionProperty::GetSystemBarProperty() const
{
    return sysBarPropMap_;
}

void WindowSessionProperty::SetWindowLimits(const WindowLimits& windowLimits)
{
    limits_ = windowLimits;
}

WindowLimits WindowSessionProperty::GetWindowLimits() const
{
    return limits_;
}

void WindowSessionProperty::SetWindowLimitsVP(const WindowLimits& windowLimits)
{
    limitsVP_ = windowLimits;
}

WindowLimits WindowSessionProperty::GetWindowLimitsVP() const
{
    return limitsVP_;
}

void WindowSessionProperty::SetWindowMode(WindowMode mode)
{
    windowMode_ = mode;
}

WindowMode WindowSessionProperty::GetWindowMode() const
{
    return windowMode_;
}

WindowState WindowSessionProperty::GetWindowState() const
{
    return windowState_;
}

void WindowSessionProperty::SetWindowState(WindowState state)
{
    windowState_ = state;
}

void WindowSessionProperty::SetKeyboardLayoutParams(const KeyboardLayoutParams& params)
{
    std::lock_guard<std::mutex> lock(keyboardParamsMutex_);
    keyboardLayoutParams_.gravity_ = params.gravity_;
    keyboardLayoutParams_.landscapeAvoidHeight_ = params.landscapeAvoidHeight_;
    keyboardLayoutParams_.portraitAvoidHeight_ = params.portraitAvoidHeight_;
    keyboardLayoutParams_.LandscapeKeyboardRect_ = params.LandscapeKeyboardRect_;
    keyboardLayoutParams_.PortraitKeyboardRect_ = params.PortraitKeyboardRect_;
    keyboardLayoutParams_.LandscapePanelRect_ = params.LandscapePanelRect_;
    keyboardLayoutParams_.PortraitPanelRect_ = params.PortraitPanelRect_;
    keyboardLayoutParams_.displayId_ = params.displayId_;
}

KeyboardLayoutParams WindowSessionProperty::GetKeyboardLayoutParams() const
{
    std::lock_guard<std::mutex> lock(keyboardParamsMutex_);
    return keyboardLayoutParams_;
}

void WindowSessionProperty::AddKeyboardLayoutParams(const uint64_t screenId, const KeyboardLayoutParams& params)
{
    std::lock_guard<std::mutex> lock(keyboardParamsMutex_);
    if (screenId != DISPLAY_ID_INVALID) {
        keyboardLayoutParamsMap_[screenId] = params;
    }
}

void WindowSessionProperty::ClearCachedKeyboardParamsOnScreenDisconnected(const uint64_t screenId)
{
    std::lock_guard<std::mutex> lock(keyboardParamsMutex_);
    auto it = keyboardLayoutParamsMap_.find(screenId);
    if (it == keyboardLayoutParamsMap_.end()) {
        return;
    }
    keyboardLayoutParamsMap_.erase(it);
}

void WindowSessionProperty::GetKeyboardLayoutParamsByScreenId(
    const uint64_t screenId, KeyboardLayoutParams& keyboardLayoutParams)
{
    std::lock_guard<std::mutex> lock(keyboardParamsMutex_);
    auto it = keyboardLayoutParamsMap_.find(screenId);
    if (it == keyboardLayoutParamsMap_.end()) {
        TLOGW(WmsLogTag::WMS_KEYBOARD, "Get keyboardParams failed, %{public}" PRIu64, screenId);
        return;
    }
    const KeyboardLayoutParams& params = it->second;
    keyboardLayoutParams.gravity_ = params.gravity_;
    keyboardLayoutParams.landscapeAvoidHeight_ = params.landscapeAvoidHeight_;
    keyboardLayoutParams.portraitAvoidHeight_ = params.portraitAvoidHeight_;
    keyboardLayoutParams.LandscapeKeyboardRect_ = params.LandscapeKeyboardRect_;
    keyboardLayoutParams.PortraitKeyboardRect_ = params.PortraitKeyboardRect_;
    keyboardLayoutParams.LandscapePanelRect_ = params.LandscapePanelRect_;
    keyboardLayoutParams.PortraitPanelRect_ = params.PortraitPanelRect_;
    keyboardLayoutParams.displayId_ = params.displayId_;
    TLOGI(WmsLogTag::WMS_KEYBOARD, "Get keyboardParams: %{public}u|%{public}d|%{public}d|%{public}s|%{public}s|"
        "%{public}s|%{public}s|%{public}" PRIu64, static_cast<uint32_t>(params.gravity_),
        params.landscapeAvoidHeight_, params.portraitAvoidHeight_, params.LandscapeKeyboardRect_.ToString().c_str(),
        params.PortraitKeyboardRect_.ToString().c_str(), params.LandscapePanelRect_.ToString().c_str(),
        params.PortraitPanelRect_.ToString().c_str(), params.displayId_);
}

void WindowSessionProperty::SetDecorEnable(bool isDecorEnable)
{
    isDecorEnable_ = isDecorEnable;
}

bool WindowSessionProperty::IsDecorEnable()
{
    return isDecorEnable_;
}

void WindowSessionProperty::SetWindowModeSupportType(uint32_t windowModeSupportType)
{
    windowModeSupportType_ = windowModeSupportType;
}

uint32_t WindowSessionProperty::GetWindowModeSupportType() const
{
    return windowModeSupportType_;
}

void WindowSessionProperty::SetSupportedWindowModes(
    const std::vector<AppExecFwk::SupportWindowMode>& supportedWindowModes)
{
    std::lock_guard<std::mutex> lock(supportWindowModesMutex_);
    supportedWindowModes_ = supportedWindowModes;
}

void WindowSessionProperty::GetSupportedWindowModes(
    std::vector<AppExecFwk::SupportWindowMode>& supportedWindowModes) const
{
    std::lock_guard<std::mutex> lock(supportWindowModesMutex_);
    supportedWindowModes = supportedWindowModes_;
}

void WindowSessionProperty::SetWindowSizeLimits(const WindowSizeLimits& windowSizeLimits)
{
    windowSizeLimits_ = windowSizeLimits;
}

WindowSizeLimits WindowSessionProperty::GetWindowSizeLimits() const
{
    return windowSizeLimits_;
}

void WindowSessionProperty::SetAnimationFlag(uint32_t animationFlag)
{
    animationFlag_ = animationFlag;
}

uint32_t WindowSessionProperty::GetAnimationFlag() const
{
    return animationFlag_;
}

bool WindowSessionProperty::IsFloatingWindowAppType() const
{
    return isFloatingWindowAppType_;
}

void WindowSessionProperty::setTouchHotAreasInner(const std::vector<Rect>& rects, std::vector<Rect>& touchHotAreas)
{
    if (GetPersistentId() != 0 && rects != touchHotAreas) {
        std::ostringstream oss;
        for (const auto& rect : rects) {
            oss << "[" << rect.posX_ << "," << rect.posY_ << "," << rect.width_ << "," << rect.height_ << "]";
        }
        TLOGI(WmsLogTag::WMS_EVENT, "id:%{public}d hot:%{public}s", GetPersistentId(), oss.str().c_str());
    }
    touchHotAreas = rects;
}

void WindowSessionProperty::SetTouchHotAreas(const std::vector<Rect>& rects)
{
    {
        std::lock_guard lock(touchHotAreasMutex_);
        setTouchHotAreasInner(rects, touchHotAreas_);
    }
    if (touchHotAreasChangeCallback_) {
        touchHotAreasChangeCallback_();
    }
}

void WindowSessionProperty::SetKeyboardTouchHotAreas(const KeyboardTouchHotAreas& keyboardTouchHotAreas)
{
    {
        std::lock_guard lock(touchHotAreasMutex_);
        setTouchHotAreasInner(
            keyboardTouchHotAreas.landscapeKeyboardHotAreas_, keyboardTouchHotAreas_.landscapeKeyboardHotAreas_);
        setTouchHotAreasInner(
            keyboardTouchHotAreas.portraitKeyboardHotAreas_, keyboardTouchHotAreas_.portraitKeyboardHotAreas_);
        setTouchHotAreasInner(
            keyboardTouchHotAreas.landscapePanelHotAreas_, keyboardTouchHotAreas_.landscapePanelHotAreas_);
        setTouchHotAreasInner(
            keyboardTouchHotAreas.portraitPanelHotAreas_, keyboardTouchHotAreas_.portraitPanelHotAreas_);
        keyboardTouchHotAreas_.displayId_ = keyboardTouchHotAreas.displayId_;
    }
    if (touchHotAreasChangeCallback_) {
        touchHotAreasChangeCallback_();
    }
}

void WindowSessionProperty::GetTouchHotAreas(std::vector<Rect>& rects) const
{
    std::lock_guard lock(touchHotAreasMutex_);
    rects = touchHotAreas_;
}

KeyboardTouchHotAreas WindowSessionProperty::GetKeyboardTouchHotAreas() const
{
    std::lock_guard lock(touchHotAreasMutex_);
    return keyboardTouchHotAreas_;
}

void WindowSessionProperty::KeepKeyboardOnFocus(bool keepKeyboardFlag)
{
    keepKeyboardFlag_ = keepKeyboardFlag;
}

bool WindowSessionProperty::GetKeepKeyboardFlag() const
{
    return keepKeyboardFlag_;
}

void WindowSessionProperty::SetCallingSessionId(uint32_t sessionId)
{
    callingSessionId_ = sessionId;
}

uint32_t WindowSessionProperty::GetCallingSessionId() const
{
    return callingSessionId_;
}

void WindowSessionProperty::SetPiPTemplateInfo(const PiPTemplateInfo& pipTemplateInfo)
{
    pipTemplateInfo_ = pipTemplateInfo;
}

PiPTemplateInfo WindowSessionProperty::GetPiPTemplateInfo() const
{
    return pipTemplateInfo_;
}

void WindowSessionProperty::SetFbTemplateInfo(const FloatingBallTemplateInfo& fbTemplateInfo)
{
    std::lock_guard<std::mutex> lock(fbTemplateMutex_);
    fbTemplateInfo_ = fbTemplateInfo;
}

FloatingBallTemplateInfo WindowSessionProperty::GetFbTemplateInfo() const
{
    std::lock_guard<std::mutex> lock(fbTemplateMutex_);
    return fbTemplateInfo_;
}

void WindowSessionProperty::SetIsNeedUpdateWindowMode(bool isNeedUpdateWindowMode)
{
    isNeedUpdateWindowMode_ = isNeedUpdateWindowMode;
}

bool WindowSessionProperty::GetIsNeedUpdateWindowMode() const
{
    return isNeedUpdateWindowMode_;
}

void WindowSessionProperty::SetWindowCornerRadius(float cornerRadius)
{
    std::lock_guard<std::mutex> lock(cornerRadiusMutex_);
    cornerRadius_ = cornerRadius;
}

float WindowSessionProperty::GetWindowCornerRadius() const
{
    std::lock_guard<std::mutex> lock(cornerRadiusMutex_);
    return cornerRadius_;
}

void WindowSessionProperty::SetWindowShadows(const ShadowsInfo& shadowsInfo)
{
    std::lock_guard<std::mutex> lock(shadowsInfoMutex_);
    shadowsInfo_ = shadowsInfo;
}

ShadowsInfo WindowSessionProperty::GetWindowShadows() const
{
    std::lock_guard<std::mutex> lock(shadowsInfoMutex_);
    return shadowsInfo_;
}

void WindowSessionProperty::SetUseControlState(bool isUseControlState)
{
    std::lock_guard<std::mutex> lock(lifecycleUseControlMutex_);
    isUseControlState_ = isUseControlState;
}

bool WindowSessionProperty::GetUseControlState() const
{
    std::lock_guard<std::mutex> lock(lifecycleUseControlMutex_);
    return isUseControlState_;
}

void WindowSessionProperty::SetRotationLocked(bool locked)
{
    isRotationLock_ = locked;
}
 
bool WindowSessionProperty::GetRotationLocked() const
{
    return isRotationLock_;
}

bool WindowSessionProperty::MarshallingWindowLimits(Parcel& parcel) const
{
    auto writeWindowLimits = [&parcel](const WindowLimits& limits) -> bool {
        return parcel.WriteUint32(limits.maxWidth_) &&
               parcel.WriteUint32(limits.maxHeight_) &&
               parcel.WriteUint32(limits.minWidth_) &&
               parcel.WriteUint32(limits.minHeight_) &&
               parcel.WriteFloat(limits.maxRatio_) &&
               parcel.WriteFloat(limits.minRatio_) &&
               parcel.WriteFloat(limits.vpRatio_) &&
               parcel.WriteUint32(static_cast<uint32_t>(limits.pixelUnit_));
    };

    return writeWindowLimits(limits_) &&
           writeWindowLimits(limitsVP_) &&
           writeWindowLimits(userLimits_);
}

void WindowSessionProperty::UnmarshallingWindowLimits(Parcel& parcel, WindowSessionProperty* property)
{
    auto readWindowLimits = [&parcel]() -> WindowLimits {
        return {
            parcel.ReadUint32(),  // maxWidth
            parcel.ReadUint32(),  // maxHeight
            parcel.ReadUint32(),  // minWidth
            parcel.ReadUint32(),  // minHeight
            parcel.ReadFloat(),   // maxRatio
            parcel.ReadFloat(),   // minRatio
            parcel.ReadFloat(),   // vpRatio
            static_cast<PixelUnit>(parcel.ReadUint32())  // pixelUnit
        };
    };

    property->SetWindowLimits(readWindowLimits());
    property->SetWindowLimitsVP(readWindowLimits());
    property->SetUserWindowLimits(readWindowLimits());
}

bool WindowSessionProperty::MarshallingSystemBarMap(Parcel& parcel) const
{
    auto size = sysBarPropMap_.size();
    uint32_t maxSystemBarNumber = 3;
    if (size > maxSystemBarNumber) { // max systembar number
        return false;
    }

    if (!parcel.WriteUint32(static_cast<uint32_t>(size))) {
        return false;
    }
    for (auto it : sysBarPropMap_) {
        if (!parcel.WriteUint32(static_cast<uint32_t>(it.first))) {
            return false;
        }
        if (!(parcel.WriteBool(it.second.enable_) && parcel.WriteUint32(it.second.backgroundColor_) &&
              parcel.WriteUint32(it.second.contentColor_) && parcel.WriteBool(it.second.enableAnimation_) &&
              parcel.WriteUint32(static_cast<uint32_t>(it.second.settingFlag_)))) {
            return false;
        }
    }
    return true;
}

void WindowSessionProperty::UnMarshallingSystemBarMap(Parcel& parcel, WindowSessionProperty* property)
{
    uint32_t size = parcel.ReadUint32();
    uint32_t maxSystemBarNumber = 3;
    if (size > maxSystemBarNumber) { // max systembar number
        return;
    }

    for (uint32_t i = 0; i < size; i++) {
        WindowType type = static_cast<WindowType>(parcel.ReadUint32());
        SystemBarProperty prop = { parcel.ReadBool(), parcel.ReadUint32(), parcel.ReadUint32(), parcel.ReadBool(),
            static_cast<SystemBarSettingFlag>(parcel.ReadUint32()) };
        property->SetSystemBarProperty(type, prop);
    }
}

bool WindowSessionProperty::MarshallingTouchHotAreasInner(const std::vector<Rect>& touchHotAreas, Parcel& parcel) const
{
    auto size = touchHotAreas.size();
    if (size > TOUCH_HOT_AREA_MAX_NUM) {
        return false;
    }
    if (!parcel.WriteUint32(static_cast<uint32_t>(size))) {
        return false;
    }
    for (const auto& rect : touchHotAreas) {
        if (!parcel.WriteInt32(rect.posX_) || !parcel.WriteInt32(rect.posY_) ||
            !parcel.WriteUint32(rect.width_) || !parcel.WriteUint32(rect.height_)) {
            return false;
        }
    }
    return true;
}

bool WindowSessionProperty::MarshallingTouchHotAreas(Parcel& parcel) const
{
    return MarshallingTouchHotAreasInner(touchHotAreas_, parcel);
}

bool WindowSessionProperty::MarshallingKeyboardTouchHotAreas(Parcel& parcel) const
{
    return MarshallingTouchHotAreasInner(keyboardTouchHotAreas_.landscapeKeyboardHotAreas_, parcel) &&
           MarshallingTouchHotAreasInner(keyboardTouchHotAreas_.portraitKeyboardHotAreas_, parcel) &&
           MarshallingTouchHotAreasInner(keyboardTouchHotAreas_.landscapePanelHotAreas_, parcel) &&
           MarshallingTouchHotAreasInner(keyboardTouchHotAreas_.portraitPanelHotAreas_, parcel) &&
           parcel.WriteUint64(keyboardTouchHotAreas_.displayId_);
}

void WindowSessionProperty::UnmarshallingTouchHotAreasInner(Parcel& parcel, std::vector<Rect>& touchHotAreas)
{
    uint32_t size = parcel.ReadUint32();
    if (size > TOUCH_HOT_AREA_MAX_NUM) {
        return;
    }
    for (uint32_t i = 0; i < size; i++) {
        touchHotAreas.emplace_back(
            Rect{ parcel.ReadInt32(), parcel.ReadInt32(), parcel.ReadUint32(), parcel.ReadUint32() });
    }
}

void WindowSessionProperty::UnmarshallingTouchHotAreas(Parcel& parcel, WindowSessionProperty* property)
{
    UnmarshallingTouchHotAreasInner(parcel, property->touchHotAreas_);
}

void WindowSessionProperty::UnmarshallingKeyboardTouchHotAreas(Parcel& parcel, WindowSessionProperty* property)
{
    UnmarshallingTouchHotAreasInner(parcel, property->keyboardTouchHotAreas_.landscapeKeyboardHotAreas_);
    UnmarshallingTouchHotAreasInner(parcel, property->keyboardTouchHotAreas_.portraitKeyboardHotAreas_);
    UnmarshallingTouchHotAreasInner(parcel, property->keyboardTouchHotAreas_.landscapePanelHotAreas_);
    UnmarshallingTouchHotAreasInner(parcel, property->keyboardTouchHotAreas_.portraitPanelHotAreas_);
    property->keyboardTouchHotAreas_.displayId_ = parcel.ReadUint64();
}

bool WindowSessionProperty::MarshallingPiPTemplateInfo(Parcel& parcel) const
{
    if (!WindowHelper::IsPipWindow(type_)) {
        return true;
    }
    return parcel.WriteParcelable(&pipTemplateInfo_);
}

void WindowSessionProperty::UnmarshallingPiPTemplateInfo(Parcel& parcel, WindowSessionProperty* property)
{
    if (!WindowHelper::IsPipWindow(property->GetWindowType())) {
        return;
    }
    sptr<PiPTemplateInfo> pipTemplateInfo = parcel.ReadParcelable<PiPTemplateInfo>();
    if (pipTemplateInfo == nullptr) {
        return;
    }
    property->SetPiPTemplateInfo(*pipTemplateInfo);
}

bool WindowSessionProperty::MarshallingFbTemplateInfo(Parcel& parcel) const
{
    if (!WindowHelper::IsFbWindow(type_)) {
        return true;
    }
    return parcel.WriteParcelable(&fbTemplateInfo_);
}

void WindowSessionProperty::UnmarshallingFbTemplateInfo(Parcel& parcel, WindowSessionProperty* property)
{
    if (!WindowHelper::IsFbWindow(property->GetWindowType())) {
        return;
    }
    sptr<FloatingBallTemplateInfo> fbTemplateInfo = parcel.ReadParcelable<FloatingBallTemplateInfo>();
    if (fbTemplateInfo == nullptr) {
        return;
    }
    property->SetFbTemplateInfo(*fbTemplateInfo);
}

bool WindowSessionProperty::MarshallingWindowMask(Parcel& parcel) const
{
    if (!parcel.WriteBool(isShaped_)) {
        return false;
    }
    if (isShaped_) {
        std::lock_guard<std::mutex> lock(windowMaskMutex_);
        if (!windowMask_->Marshalling(parcel)) {
            return false;
        }
    }
    return true;
}

void WindowSessionProperty::UnmarshallingWindowMask(Parcel& parcel, WindowSessionProperty* property)
{
    bool isShaped = parcel.ReadBool();
    property->SetIsShaped(isShaped);
    if (isShaped) {
        Media::PixelMap* windowMask = Media::PixelMap::Unmarshalling(parcel);
        if (windowMask != nullptr) {
            property->SetWindowMask(std::shared_ptr<Media::PixelMap>(windowMask));
        }
    }
}

bool WindowSessionProperty::MarshallingMainWindowTopmost(Parcel& parcel) const
{
    if (!parcel.WriteBool(mainWindowTopmost_) || !parcel.WriteUint32(accessTokenId_)) {
        return false;
    }
    return true;
}

void WindowSessionProperty::UnmarshallingMainWindowTopmost(Parcel& parcel, WindowSessionProperty* property)
{
    property->SetMainWindowTopmost(parcel.ReadBool());
    property->SetAccessTokenId(parcel.ReadUint32());
}

bool WindowSessionProperty::MarshallingSessionInfo(Parcel& parcel) const
{
    if (!parcel.WriteString(sessionInfo_.bundleName_) || !parcel.WriteString(sessionInfo_.moduleName_) ||
        !parcel.WriteString(sessionInfo_.abilityName_) || !parcel.WriteInt32(sessionInfo_.currentRotation_) ||
        !parcel.WriteInt32(static_cast<int32_t>(sessionInfo_.continueState))) {
        return false;
    }
    const auto& want = sessionInfo_.want;
    bool hasWant = want != nullptr;
    if (!parcel.WriteBool(hasWant)) {
        return false;
    }
    if (hasWant && !parcel.WriteParcelable(want.get())) {
        return false;
    }
    if (!parcel.WriteBool(sessionInfo_.isFollowParentMultiScreenPolicy)) {
        return false;
    }
    if (!parcel.WriteBool(sessionInfo_.isKeyboardWillShowRegistered_) ||
        !parcel.WriteBool(sessionInfo_.isKeyboardWillHideRegistered_) ||
        !parcel.WriteBool(sessionInfo_.isKeyboardDidShowRegistered_) ||
        !parcel.WriteBool(sessionInfo_.isKeyboardDidHideRegistered_)) {
        return false;
    }
    if (!parcel.WriteBool(sessionInfo_.isReceiveDragEventEnabled_)) {
        return false;
    }
    if (!parcel.WriteBool(sessionInfo_.isSeparationTouchEnabled_)) {
        return false;
    }
    return true;
}

bool WindowSessionProperty::UnmarshallingSessionInfo(Parcel& parcel, WindowSessionProperty* property)
{
    std::string bundleName;
    std::string moduleName;
    std::string abilityName;
    if (!parcel.ReadString(bundleName) || !parcel.ReadString(moduleName) || !parcel.ReadString(abilityName)) {
        TLOGE(WmsLogTag::DEFAULT, "Failed to read String!");
        return false;
    }
    SessionInfo info = { bundleName, moduleName, abilityName };
    int32_t currentRotation;
    if (!parcel.ReadInt32(currentRotation)) {
        TLOGE(WmsLogTag::DEFAULT, "Failed to read currentRotation!");
        return false;
    }
    info.currentRotation_ = currentRotation;
    int32_t continueState;
    if (!parcel.ReadInt32(continueState)) {
        TLOGE(WmsLogTag::DEFAULT, "Failed to read continueState!");
        return false;
    }
    info.continueState = static_cast<ContinueState>(continueState);
    bool hasWant;
    if (!parcel.ReadBool(hasWant)) {
        TLOGE(WmsLogTag::DEFAULT, "Failed to read hasWant!");
        return false;
    }
    if (hasWant) {
        std::shared_ptr<AAFwk::Want> want(parcel.ReadParcelable<AAFwk::Want>());
        if (want == nullptr) {
            TLOGE(WmsLogTag::DEFAULT, "Failed to read want!");
            return false;
        }
        info.want = want;
    }
    bool isFollowParentMultiScreenPolicy = false;
    if (!parcel.ReadBool(isFollowParentMultiScreenPolicy)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to read isFollowParentMultiScreenPolicy!");
        return false;
    }
    info.isFollowParentMultiScreenPolicy = isFollowParentMultiScreenPolicy;
    if (!parcel.ReadBool(info.isKeyboardWillShowRegistered_) ||
        !parcel.ReadBool(info.isKeyboardWillHideRegistered_) ||
        !parcel.ReadBool(info.isKeyboardDidShowRegistered_) ||
        !parcel.ReadBool(info.isKeyboardDidHideRegistered_)) {
        TLOGE(WmsLogTag::DEFAULT, "Failed to read keyboard registered state!");
        return false;
    }
    if (!parcel.ReadBool(info.isReceiveDragEventEnabled_)) {
        TLOGE(WmsLogTag::WMS_EVENT, "Failed to read isReceiveDragEventEnabled!");
        return false;
    }
    if (!parcel.ReadBool(info.isSeparationTouchEnabled_)) {
        TLOGE(WmsLogTag::WMS_EVENT, "Failed to read isSeparationTouchEnabled!");
        return false;
    }
    property->SetSessionInfo(info);
    return true;
}

bool WindowSessionProperty::MarshallingTransitionAnimationMap(Parcel& parcel) const
{
    uint32_t transitionAnimationMapSize = transitionAnimationConfig_.size();
    if (transitionAnimationMapSize > TRANSITION_ANIMATION_MAP_SIZE_MAX_NUM ||
        !parcel.WriteUint32(transitionAnimationMapSize)) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Failed to write transitionAnimationMapSize");
        return false;
    }
    for (const auto& [transitionType, animation] : transitionAnimationConfig_) {
        if (!parcel.WriteUint32(static_cast<uint32_t>(transitionType))) {
            TLOGE(WmsLogTag::WMS_ANIMATION, "Failed to write transitionType");
            return false;
        }
        if (animation == nullptr || !parcel.WriteParcelable(animation.get())) {
            TLOGE(WmsLogTag::WMS_ANIMATION, "Failed to write transitionAnimation");
            return false;
        }
    }
    return true;
}

bool WindowSessionProperty::UnmarshallingTransitionAnimationMap(Parcel& parcel, WindowSessionProperty* property)
{
    uint32_t transitionAnimationMapSize = 0;
    if (!parcel.ReadUint32(transitionAnimationMapSize) ||
        transitionAnimationMapSize > TRANSITION_ANIMATION_MAP_SIZE_MAX_NUM) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Failed to read transitionAnimationMapSize");
        return false;
    }
    uint32_t transitionType = 0;
    std::shared_ptr<TransitionAnimation> animation = nullptr;
    for (uint32_t i = 0; i < transitionAnimationMapSize; ++i) {
        if (!parcel.ReadUint32(transitionType)) {
            TLOGE(WmsLogTag::WMS_ANIMATION, "Failed to read transitionType");
            return false;
        }
        animation = std::shared_ptr<TransitionAnimation>(parcel.ReadParcelable<TransitionAnimation>());
        if (animation == nullptr) {
            TLOGE(WmsLogTag::WMS_ANIMATION, "Failed to read transitionAnimation");
            return false;
        }
        property->transitionAnimationConfig_[static_cast<WindowTransitionType>(transitionType)] = animation;
    }
    return true;
}

void WindowSessionProperty::SetIsAppSupportPhoneInPc(bool isSupportPhone)
{
    isAppSupportPhoneInPc_ = isSupportPhone;
}

bool WindowSessionProperty::GetIsAppSupportPhoneInPc() const
{
    return isAppSupportPhoneInPc_;
}

void WindowSessionProperty::SetIsPcAppInPad(bool isPcAppInLargeScreenDevice)
{
    isPcAppInLargeScreenDevice_ = isPcAppInLargeScreenDevice;
}

bool WindowSessionProperty::GetIsPcAppInPad() const
{
    return isPcAppInLargeScreenDevice_;
}

void WindowSessionProperty::SetSubWindowLevel(uint32_t subWindowLevel)
{
    subWindowLevel_ = subWindowLevel;
}

uint32_t WindowSessionProperty::GetSubWindowLevel() const
{
    return subWindowLevel_;
}

void WindowSessionProperty::SetSubWindowZLevel(int32_t zLevel)
{
    zLevel_ = zLevel;
}

int32_t WindowSessionProperty::GetSubWindowZLevel() const
{
    return zLevel_;
}

void WindowSessionProperty::SetWindowAnchorInfo(const WindowAnchorInfo& windowAnchorInfo)
{
    windowAnchorInfo_ = windowAnchorInfo;
}

WindowAnchorInfo WindowSessionProperty::GetWindowAnchorInfo() const
{
    return windowAnchorInfo_;
}

void WindowSessionProperty::SetZIndex(int32_t zIndex)
{
    zIndex_ = zIndex;
}

int32_t WindowSessionProperty::GetZIndex() const
{
    return zIndex_;
}

void WindowSessionProperty::SetIsAtomicService(bool isAtomicService)
{
    std::lock_guard lock(atomicServiceMutex_);
    isAtomicService_ = isAtomicService;
}

bool WindowSessionProperty::GetIsAtomicService() const
{
    std::lock_guard lock(atomicServiceMutex_);
    return isAtomicService_;
}

void WindowSessionProperty::SetSubWindowOutlineEnabled(bool subWindowOutlineEnabled)
{
    subWindowOutlineEnabled_ = subWindowOutlineEnabled;
}

bool WindowSessionProperty::IsSubWindowOutlineEnabled() const
{
    return subWindowOutlineEnabled_;
}

bool WindowSessionProperty::Marshalling(Parcel& parcel) const
{
    auto globalDisplayRect = GetGlobalDisplayRect();
    return parcel.WriteString(windowName_) && parcel.WriteInt32(windowRect_.posX_) &&
        parcel.WriteInt32(windowRect_.posY_) && parcel.WriteUint32(windowRect_.width_) &&
        parcel.WriteUint32(windowRect_.height_) && parcel.WriteInt32(requestRect_.posX_) &&
        parcel.WriteInt32(requestRect_.posY_) && parcel.WriteUint32(requestRect_.width_) &&
        parcel.WriteUint32(requestRect_.height_) && parcel.WriteInt32(globalDisplayRect.posX_) &&
        parcel.WriteInt32(globalDisplayRect.posY_) && parcel.WriteUint32(globalDisplayRect.width_) &&
        parcel.WriteUint32(globalDisplayRect.height_) &&
        parcel.WriteUint32(rectAnimationConfig_.duration) && parcel.WriteFloat(rectAnimationConfig_.x1) &&
        parcel.WriteFloat(rectAnimationConfig_.y1) && parcel.WriteFloat(rectAnimationConfig_.x2) &&
        parcel.WriteFloat(rectAnimationConfig_.y2) &&
        parcel.WriteUint32(static_cast<uint32_t>(type_)) &&
        parcel.WriteBool(focusable_) && parcel.WriteBool(focusableOnShow_) &&
        parcel.WriteBool(touchable_) && parcel.WriteBool(tokenState_) &&
        parcel.WriteBool(turnScreenOn_) && parcel.WriteBool(keepScreenOn_) && parcel.WriteBool(viewKeepScreenOn_) &&
        parcel.WriteBool(isPrivacyMode_) && parcel.WriteBool(isSystemPrivacyMode_) &&
        parcel.WriteBool(isSnapshotSkip_) && parcel.WriteBool(windowShadowEnabled_) &&
        parcel.WriteUint64(displayId_) && parcel.WriteInt32(persistentId_) &&
        MarshallingSessionInfo(parcel) &&
        MarshallingTransitionAnimationMap(parcel) &&
        parcel.WriteInt32(parentPersistentId_) && parcel.WriteBool(isFollowParentWindowDisplayId_) &&
        parcel.WriteUint32(accessTokenId_) && parcel.WriteUint32(static_cast<uint32_t>(maximizeMode_)) &&
        parcel.WriteUint32(static_cast<uint32_t>(requestedOrientation_)) &&
        parcel.WriteBool(needRotateAnimation_) &&
        parcel.WriteUint32(static_cast<uint32_t>(userRequestedOrientation_)) &&
        parcel.WriteBool(isSpecificSessionRequestOrientation_) &&
        parcel.WriteUint32(static_cast<uint32_t>(windowMode_)) &&
        parcel.WriteUint32(flags_) && parcel.WriteBool(raiseEnabled_) &&
        parcel.WriteBool(topmost_) && parcel.WriteBool(mainWindowTopmost_) &&
        parcel.WriteInt32(zLevel_) && parcel.WriteInt32(zIndex_) &&
        parcel.WriteBool(isDecorEnable_) && parcel.WriteBool(dragEnabled_) &&
        parcel.WriteBool(hideNonSystemFloatingWindows_) && parcel.WriteBool(forceHide_) &&
        MarshallingWindowLimits(parcel) && parcel.WriteFloat(brightness_) &&
        MarshallingSystemBarMap(parcel) && parcel.WriteUint32(animationFlag_) &&
        MarshallingPiPTemplateInfo(parcel) &&
        parcel.WriteBool(isFloatingWindowAppType_) && MarshallingTouchHotAreas(parcel) &&
        parcel.WriteBool(isSystemCalling_) &&
        parcel.WriteDouble(textFieldPositionY_) && parcel.WriteDouble(textFieldHeight_) &&
        parcel.WriteUint32(static_cast<uint32_t>(windowState_)) &&
        parcel.WriteBool(isNeedUpdateWindowMode_) && parcel.WriteUint32(callingSessionId_) &&
        parcel.WriteBool(isLayoutFullScreen_) &&
        parcel.WriteInt32(realParentId_) &&
        parcel.WriteBool(isUIExtFirstSubWindow_) &&
        parcel.WriteBool(isUIExtensionAbilityProcess_) &&
        parcel.WriteUint32(static_cast<uint32_t>(uiExtensionUsage_)) &&
        parcel.WriteUint32(static_cast<uint32_t>(parentWindowType_)) &&
        MarshallingWindowMask(parcel) &&
        parcel.WriteParcelable(&keyboardLayoutParams_) &&
        parcel.WriteBool(isAppSupportPhoneInPc_) &&
        parcel.WriteBool(isPcAppInLargeScreenDevice_) &&
        parcel.WriteString(appInstanceKey_) &&
        parcel.WriteInt32(appIndex_) &&
        parcel.WriteBool(isSystemKeyboard_) &&
        parcel.WriteUint32(avoidAreaOption_) && parcel.WriteBool(isWindowDelayRaiseEnabled_) &&
        parcel.WriteUint8(backgroundAlpha_) && parcel.WriteParcelable(&keyboardEffectOption_) &&
        parcel.WriteFloat(cornerRadius_) && parcel.WriteBool(isExclusivelyHighlighted_) &&
        parcel.WriteBool(isAtomicService_) && parcel.WriteUint32(apiVersion_) &&
        parcel.WriteBool(isFullScreenWaterfallMode_) && parcel.WriteBool(isAbilityHookOff_) &&
        parcel.WriteBool(isAbilityHook_) && parcel.WriteBool(isFollowScreenChange_) &&
        parcel.WriteParcelable(compatibleModeProperty_) && parcel.WriteBool(subWindowOutlineEnabled_) &&
        parcel.WriteUint32(windowModeSupportType_) &&
        MarshallingShadowsInfo(parcel) &&
        MarshallingFbTemplateInfo(parcel) &&
        MarshallingWindowAnchorInfo(parcel) &&
        parcel.WriteBool(isPcAppInpadSpecificSystemBarInvisible_) &&
        parcel.WriteBool(isPcAppInpadOrientationLandscape_) &&
        parcel.WriteBool(isPcAppInpadCompatibleMode_) &&
        parcel.WriteString(ancoRealBundleName_) &&
        parcel.WriteBool(isShowDecorInFreeMultiWindow_) &&
        parcel.WriteBool(isMobileAppInPadLayoutFullScreen_) &&
        parcel.WriteBool(isFullScreenInForceSplitMode_) &&
        parcel.WriteInt32(static_cast<int32_t>(pageCompatibleMode_)) &&
        parcel.WriteFloat(aspectRatio_) &&
        parcel.WriteBool(isRotationLock_) &&
        parcel.WriteInt32(frameNum_) &&
        parcel.WriteBool(isPrelaunch_);
}

WindowSessionProperty* WindowSessionProperty::Unmarshalling(Parcel& parcel)
{
    WindowSessionProperty* property = new(std::nothrow) WindowSessionProperty();
    if (property == nullptr) {
        return nullptr;
    }
    property->SetWindowName(parcel.ReadString());
    Rect rect = { parcel.ReadInt32(), parcel.ReadInt32(), parcel.ReadUint32(), parcel.ReadUint32() };
    property->SetWindowRect(rect);
    Rect reqRect = { parcel.ReadInt32(), parcel.ReadInt32(), parcel.ReadUint32(), parcel.ReadUint32() };
    property->SetRequestRect(reqRect);
    Rect globalDisplayRect = { parcel.ReadInt32(), parcel.ReadInt32(), parcel.ReadUint32(), parcel.ReadUint32() };
    property->SetGlobalDisplayRect(globalDisplayRect);
    RectAnimationConfig rectAnimationConfig = { parcel.ReadUint32(), parcel.ReadFloat(),
        parcel.ReadFloat(), parcel.ReadFloat(), parcel.ReadFloat() };
    property->SetRectAnimationConfig(rectAnimationConfig);
    property->SetWindowType(static_cast<WindowType>(parcel.ReadUint32()));
    property->SetFocusable(parcel.ReadBool());
    property->SetFocusableOnShow(parcel.ReadBool());
    property->SetTouchable(parcel.ReadBool());
    property->SetTokenState(parcel.ReadBool());
    property->SetTurnScreenOn(parcel.ReadBool());
    property->SetKeepScreenOn(parcel.ReadBool());
    property->SetViewKeepScreenOn(parcel.ReadBool());
    property->SetPrivacyMode(parcel.ReadBool());
    property->SetSystemPrivacyMode(parcel.ReadBool());
    property->SetSnapshotSkip(parcel.ReadBool());
    property->SetWindowShadowEnabled(parcel.ReadBool());
    property->SetDisplayId(parcel.ReadUint64());
    property->SetPersistentId(parcel.ReadInt32());
    if (!UnmarshallingSessionInfo(parcel, property)) {
        delete property;
        return nullptr;
    }
    if (!UnmarshallingTransitionAnimationMap(parcel, property)) {
        delete property;
        return nullptr;
    }
    property->SetParentPersistentId(parcel.ReadInt32());
    property->SetIsFollowParentWindowDisplayId(parcel.ReadBool());
    property->SetAccessTokenId(parcel.ReadUint32());
    property->SetMaximizeMode(static_cast<MaximizeMode>(parcel.ReadUint32()));
    property->SetRequestedOrientation(static_cast<Orientation>(parcel.ReadUint32()), parcel.ReadBool());
    property->SetUserRequestedOrientation(static_cast<Orientation>(parcel.ReadUint32()));
    property->SetIsSpecificSessionRequestOrientation(parcel.ReadBool());
    property->SetWindowMode(static_cast<WindowMode>(parcel.ReadUint32()));
    property->SetWindowFlags(parcel.ReadUint32());
    property->SetRaiseEnabled(parcel.ReadBool());
    property->SetTopmost(parcel.ReadBool());
    property->SetMainWindowTopmost(parcel.ReadBool());
    property->SetSubWindowZLevel(parcel.ReadInt32());
    property->SetZIndex(parcel.ReadInt32());
    property->SetDecorEnable(parcel.ReadBool());
    property->SetDragEnabled(parcel.ReadBool());
    property->SetHideNonSystemFloatingWindows(parcel.ReadBool());
    property->SetForceHide(parcel.ReadBool());
    UnmarshallingWindowLimits(parcel, property);
    property->SetBrightness(parcel.ReadFloat());
    UnMarshallingSystemBarMap(parcel, property);
    property->SetAnimationFlag(parcel.ReadUint32());
    UnmarshallingPiPTemplateInfo(parcel, property);
    property->SetFloatingWindowAppType(parcel.ReadBool());
    UnmarshallingTouchHotAreas(parcel, property);
    property->SetSystemCalling(parcel.ReadBool());
    property->SetTextFieldPositionY(parcel.ReadDouble());
    property->SetTextFieldHeight(parcel.ReadDouble());
    property->SetWindowState(static_cast<WindowState>(parcel.ReadUint32()));
    property->SetIsNeedUpdateWindowMode(parcel.ReadBool());
    property->SetCallingSessionId(parcel.ReadUint32());
    property->SetIsLayoutFullScreen(parcel.ReadBool());
    property->SetRealParentId(parcel.ReadInt32());
    property->SetIsUIExtFirstSubWindow(parcel.ReadBool());
    property->SetIsUIExtensionAbilityProcess(parcel.ReadBool());
    property->SetUIExtensionUsage(static_cast<UIExtensionUsage>(parcel.ReadUint32()));
    property->SetParentWindowType(static_cast<WindowType>(parcel.ReadUint32()));
    UnmarshallingWindowMask(parcel, property);
    sptr<KeyboardLayoutParams> keyboardLayoutParams = parcel.ReadParcelable<KeyboardLayoutParams>();
    if (keyboardLayoutParams == nullptr) {
        delete property;
        return nullptr;
    }
    property->SetKeyboardLayoutParams(*keyboardLayoutParams);
    property->SetIsAppSupportPhoneInPc(parcel.ReadBool());
    property->SetIsPcAppInPad(parcel.ReadBool());
    property->SetAppInstanceKey(parcel.ReadString());
    property->SetAppIndex(parcel.ReadInt32());
    property->SetIsSystemKeyboard(parcel.ReadBool());
    property->SetAvoidAreaOption(parcel.ReadUint32());
    property->SetWindowDelayRaiseEnabled(parcel.ReadBool());
    property->SetBackgroundAlpha(parcel.ReadUint8());
    sptr<KeyboardEffectOption> keyboardEffectOption = parcel.ReadParcelable<KeyboardEffectOption>();
    if (keyboardEffectOption == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Failed to read keyboardEffectOption");
        delete property;
        return nullptr;
    }
    property->SetKeyboardEffectOption(*keyboardEffectOption);
    property->SetWindowCornerRadius(parcel.ReadFloat());
    property->SetExclusivelyHighlighted(parcel.ReadBool());
    property->SetIsAtomicService(parcel.ReadBool());
    property->SetApiVersion(parcel.ReadUint32());
    property->SetIsFullScreenWaterfallMode(parcel.ReadBool());
    property->SetIsAbilityHookOff(parcel.ReadBool());
    property->SetIsAbilityHook(parcel.ReadBool());
    property->SetFollowScreenChange(parcel.ReadBool());
    property->SetCompatibleModeProperty(parcel.ReadParcelable<CompatibleModeProperty>());
    property->SetSubWindowOutlineEnabled(parcel.ReadBool());
    property->SetWindowModeSupportType(parcel.ReadUint32());
    UnmarshallingShadowsInfo(parcel, property);
    UnmarshallingFbTemplateInfo(parcel, property);
    UnmarshallingWindowAnchorInfo(parcel, property);
    property->SetPcAppInpadSpecificSystemBarInvisible(parcel.ReadBool());
    property->SetPcAppInpadOrientationLandscape(parcel.ReadBool());
    property->SetPcAppInpadCompatibleMode(parcel.ReadBool());
    property->SetAncoRealBundleName(parcel.ReadString());
    property->SetIsShowDecorInFreeMultiWindow(parcel.ReadBool());
    property->SetMobileAppInPadLayoutFullScreen(parcel.ReadBool());
    property->SetIsFullScreenInForceSplitMode(parcel.ReadBool());
    property->SetPageCompatibleMode(static_cast<CompatibleStyleMode>(parcel.ReadInt32()));
    property->SetAspectRatio(parcel.ReadFloat());
    property->SetRotationLocked(parcel.ReadBool());
    property->SetFrameNum(parcel.ReadInt32());
    property->SetPrelaunch(parcel.ReadBool());
    return property;
}

void WindowSessionProperty::CopyFrom(const sptr<WindowSessionProperty>& property)
{
    windowName_ = property->windowName_;
    sessionInfo_ = property->sessionInfo_;
    requestRect_ = property->requestRect_;
    globalDisplayRect_ = property->GetGlobalDisplayRect();
    rectAnimationConfig_ = property->rectAnimationConfig_;
    windowRect_ = property->windowRect_;
    type_ = property->type_;
    focusable_ = property->focusable_;
    focusableOnShow_ = property->focusableOnShow_;
    touchable_ = property->touchable_;
    dragEnabled_ = property->dragEnabled_;
    raiseEnabled_ = property->raiseEnabled_;
    isSystemCalling_ = property->isSystemCalling_;
    tokenState_ = property->tokenState_;
    turnScreenOn_ = property->turnScreenOn_;
    keepScreenOn_ = property->keepScreenOn_;
    viewKeepScreenOn_ = property->viewKeepScreenOn_;
    windowShadowEnabled_ = property->windowShadowEnabled_;
    topmost_ = property->topmost_;
    mainWindowTopmost_ = property->mainWindowTopmost_;
    zLevel_ = property->zLevel_;
    zIndex_ = property->zIndex_;
    requestedOrientation_ = property->requestedOrientation_;
    defaultRequestedOrientation_ = property->defaultRequestedOrientation_;
    userRequestedOrientation_ = property->userRequestedOrientation_;
    isPrivacyMode_ = property->isPrivacyMode_;
    isSystemPrivacyMode_ = property->isSystemPrivacyMode_;
    isSnapshotSkip_ = property->isSnapshotSkip_;
    brightness_ = property->brightness_;
    displayId_ = property->displayId_;
    parentId_ = property->parentId_;
    flags_ = property->flags_;
    persistentId_ = property->persistentId_;
    isFollowParentWindowDisplayId_ = property->isFollowParentWindowDisplayId_;
    parentPersistentId_ = property->parentPersistentId_;
    accessTokenId_ = property->accessTokenId_;
    maximizeMode_ = property->maximizeMode_;
    windowMode_ = property->windowMode_;
    windowState_ = property->windowState_;
    limits_ = property->limits_;
    limitsVP_ = property->limitsVP_;
    userLimits_ = property->userLimits_;
    configLimitsVP_ = property->configLimitsVP_;
    lastVpr_ = property->lastVpr_;
    pipTemplateInfo_ = property->pipTemplateInfo_;
    fbTemplateInfo_ = property->fbTemplateInfo_;
    keyboardLayoutParams_ = property->keyboardLayoutParams_;
    windowModeSupportType_ = property->windowModeSupportType_;
    sysBarPropMap_ = property->sysBarPropMap_;
    isDecorEnable_ = property->isDecorEnable_;
    animationFlag_ = property->animationFlag_;
    trans_ = property->trans_;
    isFloatingWindowAppType_ = property->isFloatingWindowAppType_;
    touchHotAreas_ = property->touchHotAreas_;
    keyboardTouchHotAreas_ = property->keyboardTouchHotAreas_;
    hideNonSystemFloatingWindows_ = property->hideNonSystemFloatingWindows_;
    isSkipSelfWhenShowOnVirtualScreen_ = property->isSkipSelfWhenShowOnVirtualScreen_;
    isSkipEventOnCastPlus_ = property->isSkipEventOnCastPlus_;
    forceHide_ = property->forceHide_;
    keepKeyboardFlag_ = property->keepKeyboardFlag_;
    callingSessionId_ = property->callingSessionId_;
    textFieldPositionY_ = property->textFieldPositionY_;
    textFieldHeight_ = property->textFieldHeight_;
    isNeedUpdateWindowMode_ = property->isNeedUpdateWindowMode_;
    touchHotAreasChangeCallback_ = property->touchHotAreasChangeCallback_;
    isLayoutFullScreen_ = property->isLayoutFullScreen_;
    isShaped_ = property->isShaped_;
    fullScreenStart_ = property->fullScreenStart_;
    {
        std::lock_guard<std::mutex> lock(windowMaskMutex_);
        windowMask_ = property->windowMask_;
    }
    collaboratorType_ = property->collaboratorType_;
    isAppSupportPhoneInPc_ = property->isAppSupportPhoneInPc_;
    isPcAppInLargeScreenDevice_ = property->isPcAppInLargeScreenDevice_;
    subWindowLevel_ = property->subWindowLevel_;
    realParentId_ = property->realParentId_;
    uiExtensionUsage_ = property->uiExtensionUsage_;
    isUIExtFirstSubWindow_ = property->isUIExtFirstSubWindow_;
    isUIExtensionAbilityProcess_ = property->isUIExtensionAbilityProcess_;
    isUIExtAnySubWindow_ = property->isUIExtAnySubWindow_;
    parentWindowType_ = property->parentWindowType_;
    appInstanceKey_ = property->appInstanceKey_;
    appIndex_ = property->appIndex_;
    isSystemKeyboard_ = property->isSystemKeyboard_;
    avoidAreaOption_ = property->avoidAreaOption_;
    isWindowDelayRaiseEnabled_ = property->isWindowDelayRaiseEnabled_;
    backgroundAlpha_ = property->backgroundAlpha_;
    keyboardEffectOption_ = property->keyboardEffectOption_;
    isExclusivelyHighlighted_ = property->isExclusivelyHighlighted_;
    cornerRadius_ = property->cornerRadius_;
    isAtomicService_ = property->isAtomicService_;
    apiVersion_ = property->apiVersion_;
    isFullScreenWaterfallMode_ = property->isFullScreenWaterfallMode_;
    isAbilityHookOff_ = property->isAbilityHookOff_;
    isAbilityHook_ = property->isAbilityHook_;
    isFollowScreenChange_ = property->isFollowScreenChange_;
    subWindowOutlineEnabled_ = property->subWindowOutlineEnabled_;
    shadowsInfo_ = property->shadowsInfo_;
    windowAnchorInfo_ = property->windowAnchorInfo_;
    isPcAppInpadSpecificSystemBarInvisible_ = property->isPcAppInpadSpecificSystemBarInvisible_;
    isPcAppInpadOrientationLandscape_ = property->isPcAppInpadOrientationLandscape_;
    isPcAppInpadCompatibleMode_ = property->isPcAppInpadCompatibleMode_;
    ancoRealBundleName_ = property->ancoRealBundleName_;
    {
        std::lock_guard<std::mutex> lock(missionInfoMutex_);
        missionInfo_ = property->missionInfo_;
    }
    isShowDecorInFreeMultiWindow_ = property->isShowDecorInFreeMultiWindow_;
    isMobileAppInPadLayoutFullScreen_ = property->isMobileAppInPadLayoutFullScreen_;
    aspectRatio_ = property->aspectRatio_;
    isRotationLock_ = property->isRotationLock_;
    statusBarHeightInImmersive_ = property->statusBarHeightInImmersive_;
    pageCompatibleMode_ = property->pageCompatibleMode_;
}

bool WindowSessionProperty::Write(Parcel& parcel, WSPropertyChangeAction action)
{
    const auto funcIter = writeFuncMap_.find(static_cast<uint64_t>(action));
    if (funcIter == writeFuncMap_.end()) {
        TLOGE(WmsLogTag::DEFAULT, "Failed to find func handler!");
        return false;
    }
    bool ret = parcel.WriteUint32(static_cast<uint32_t>(persistentId_));
    return ret && (this->*(funcIter->second))(parcel);
}

bool WindowSessionProperty::WriteActionUpdateTurnScreenOn(Parcel& parcel)
{
    return parcel.WriteBool(turnScreenOn_);
}

bool WindowSessionProperty::WriteActionUpdateKeepScreenOn(Parcel& parcel)
{
    return parcel.WriteBool(keepScreenOn_);
}

bool WindowSessionProperty::WriteActionUpdateViewKeepScreenOn(Parcel& parcel)
{
    return parcel.WriteBool(viewKeepScreenOn_);
}

bool WindowSessionProperty::WriteActionUpdateFocusable(Parcel& parcel)
{
    return parcel.WriteBool(focusable_);
}

bool WindowSessionProperty::WriteActionUpdateTouchable(Parcel& parcel)
{
    return parcel.WriteBool(touchable_);
}

bool WindowSessionProperty::WriteActionUpdateSetBrightness(Parcel& parcel)
{
    return parcel.WriteFloat(brightness_);
}

bool WindowSessionProperty::WriteActionUpdateOrientation(Parcel& parcel)
{
    return parcel.WriteUint32(static_cast<uint32_t>(requestedOrientation_)) &&
        parcel.WriteBool(needRotateAnimation_);
}

bool WindowSessionProperty::WriteActionUpdatePrivacyMode(Parcel& parcel)
{
    return parcel.WriteBool(isPrivacyMode_) && parcel.WriteBool(isSystemPrivacyMode_);
}

bool WindowSessionProperty::WriteActionUpdateSnapshotSkip(Parcel& parcel)
{
    return parcel.WriteBool(isSnapshotSkip_);
}

bool WindowSessionProperty::WriteActionUpdateMaximizeState(Parcel& parcel)
{
    return parcel.WriteUint32(static_cast<uint32_t>(maximizeMode_)) &&
        parcel.WriteBool(isLayoutFullScreen_);
}

bool WindowSessionProperty::WriteActionUpdateSystemBar(Parcel& parcel)
{
    return MarshallingSystemBarMap(parcel);
}

bool WindowSessionProperty::WriteActionUpdateFlags(Parcel& parcel)
{
    return parcel.WriteUint32(flags_);
}

bool WindowSessionProperty::WriteActionUpdateMode(Parcel& parcel)
{
    return parcel.WriteUint32(static_cast<uint32_t>(windowMode_));
}

bool WindowSessionProperty::WriteActionUpdateAnimationFlag(Parcel& parcel)
{
    return parcel.WriteUint32(animationFlag_);
}

bool WindowSessionProperty::WriteActionUpdateTouchHotArea(Parcel& parcel)
{
    return MarshallingTouchHotAreas(parcel);
}

bool WindowSessionProperty::WriteActionUpdateKeyboardTouchHotArea(Parcel& parcel)
{
    return MarshallingKeyboardTouchHotAreas(parcel);
}

bool WindowSessionProperty::WriteActionUpdateDecorEnable(Parcel& parcel)
{
    return parcel.WriteBool(isDecorEnable_);
}

bool WindowSessionProperty::WriteActionUpdateWindowLimits(Parcel& parcel)
{
    return MarshallingWindowLimits(parcel);
}

bool WindowSessionProperty::WriteActionUpdateDragenabled(Parcel& parcel)
{
    return parcel.WriteBool(dragEnabled_);
}

bool WindowSessionProperty::WriteActionUpdateRaiseenabled(Parcel& parcel)
{
    return parcel.WriteBool(raiseEnabled_);
}

bool WindowSessionProperty::WriteActionUpdateHideNonSystemFloatingWindows(Parcel& parcel)
{
    return parcel.WriteBool(hideNonSystemFloatingWindows_) &&
        parcel.WriteBool(isFloatingWindowAppType_) && parcel.WriteBool(forceHide_);
}

bool WindowSessionProperty::WriteActionUpdateTextfieldAvoidInfo(Parcel& parcel)
{
    return parcel.WriteDouble(textFieldPositionY_) && parcel.WriteDouble(textFieldHeight_);
}

bool WindowSessionProperty::WriteActionUpdateWindowMask(Parcel& parcel)
{
    return MarshallingWindowMask(parcel);
}

bool WindowSessionProperty::WriteActionUpdateTopmost(Parcel& parcel)
{
    return parcel.WriteBool(topmost_);
}

bool WindowSessionProperty::WriteActionUpdateMainWindowTopmost(Parcel& parcel)
{
    return MarshallingMainWindowTopmost(parcel);
}

bool WindowSessionProperty::WriteActionUpdateSubWindowZLevel(Parcel& parcel)
{
    return parcel.WriteInt32(zLevel_);
}

bool WindowSessionProperty::WriteActionUpdateWindowModeSupportType(Parcel& parcel)
{
    return parcel.WriteUint32(windowModeSupportType_);
}

bool WindowSessionProperty::WriteActionUpdateAvoidAreaOption(Parcel& parcel)
{
    return parcel.WriteUint32(avoidAreaOption_);
}

bool WindowSessionProperty::WriteActionUpdateBackgroundAlpha(Parcel& parcel)
{
    return parcel.WriteUint8(backgroundAlpha_);
}

bool WindowSessionProperty::WriteActionUpdateExclusivelyHighlighted(Parcel& parcel)
{
    return parcel.WriteBool(isExclusivelyHighlighted_);
}

bool WindowSessionProperty::WriteActionUpdateFollowScreenChange(Parcel& parcel)
{
    return parcel.WriteBool(isFollowScreenChange_);
}

bool WindowSessionProperty::WriteActionUpdateWindowShadowEnabled(Parcel& parcel)
{
    return parcel.WriteBool(windowShadowEnabled_);
}

bool WindowSessionProperty::WriteActionUpdateAspectRatio(Parcel& parcel)
{
    return parcel.WriteFloat(aspectRatio_);
}

bool WindowSessionProperty::WriteActionUpdateRotationLockChange(Parcel& parcel)
{
    return parcel.WriteBool(isRotationLock_);
}

void WindowSessionProperty::Read(Parcel& parcel, WSPropertyChangeAction action)
{
    const auto funcIter = readFuncMap_.find(static_cast<uint64_t>(action));
    if (funcIter == readFuncMap_.end()) {
        TLOGE(WmsLogTag::DEFAULT, "Failed to find func handler!");
        return;
    }
    SetPersistentId(parcel.ReadUint32());
    (this->*(funcIter->second))(parcel);
}

void WindowSessionProperty::ReadActionUpdateTurnScreenOn(Parcel& parcel)
{
    SetTurnScreenOn(parcel.ReadBool());
}

void WindowSessionProperty::ReadActionUpdateKeepScreenOn(Parcel& parcel)
{
    SetKeepScreenOn(parcel.ReadBool());
}

void WindowSessionProperty::ReadActionUpdateViewKeepScreenOn(Parcel& parcel)
{
    SetViewKeepScreenOn(parcel.ReadBool());
}

void WindowSessionProperty::ReadActionUpdateFocusable(Parcel& parcel)
{
    SetFocusable(parcel.ReadBool());
}

void WindowSessionProperty::ReadActionUpdateTouchable(Parcel& parcel)
{
    SetTouchable(parcel.ReadBool());
}

void WindowSessionProperty::ReadActionUpdateSetBrightness(Parcel& parcel)
{
    SetBrightness(parcel.ReadFloat());
}

void WindowSessionProperty::ReadActionUpdateOrientation(Parcel& parcel)
{
    SetRequestedOrientation(static_cast<Orientation>(parcel.ReadUint32()), parcel.ReadBool());
}

void WindowSessionProperty::ReadActionUpdatePrivacyMode(Parcel& parcel)
{
    SetPrivacyMode(parcel.ReadBool());
    SetSystemPrivacyMode(parcel.ReadBool());
}

void WindowSessionProperty::ReadActionUpdateSnapshotSkip(Parcel& parcel)
{
    SetSnapshotSkip(parcel.ReadBool());
}

void WindowSessionProperty::ReadActionUpdateMaximizeState(Parcel& parcel)
{
    SetMaximizeMode(static_cast<MaximizeMode>(parcel.ReadUint32()));
    SetIsLayoutFullScreen(parcel.ReadBool());
}

void WindowSessionProperty::ReadActionUpdateSystemBar(Parcel& parcel)
{
    UnMarshallingSystemBarMap(parcel, this);
}

void WindowSessionProperty::ReadActionUpdateFlags(Parcel& parcel)
{
    SetWindowFlags(parcel.ReadUint32());
}

void WindowSessionProperty::ReadActionUpdateMode(Parcel& parcel)
{
    SetWindowMode(static_cast<WindowMode>(parcel.ReadUint32()));
}

void WindowSessionProperty::ReadActionUpdateAnimationFlag(Parcel& parcel)
{
    SetAnimationFlag(parcel.ReadUint32());
}

void WindowSessionProperty::ReadActionUpdateTouchHotArea(Parcel& parcel)
{
    UnmarshallingTouchHotAreas(parcel, this);
}

void WindowSessionProperty::ReadActionUpdateKeyboardTouchHotArea(Parcel& parcel)
{
    UnmarshallingKeyboardTouchHotAreas(parcel, this);
}

void WindowSessionProperty::ReadActionUpdateDecorEnable(Parcel& parcel)
{
    SetDecorEnable(parcel.ReadBool());
}

void WindowSessionProperty::ReadActionUpdateWindowLimits(Parcel& parcel)
{
    UnmarshallingWindowLimits(parcel, this);
}

void WindowSessionProperty::ReadActionUpdateDragenabled(Parcel& parcel)
{
    SetDragEnabled(parcel.ReadBool());
}

void WindowSessionProperty::ReadActionUpdateRaiseenabled(Parcel& parcel)
{
    SetRaiseEnabled(parcel.ReadBool());
}

void WindowSessionProperty::ReadActionUpdateHideNonSystemFloatingWindows(Parcel& parcel)
{
    SetHideNonSystemFloatingWindows(parcel.ReadBool());
    SetFloatingWindowAppType(parcel.ReadBool());
    SetForceHide(parcel.ReadBool());
}

void WindowSessionProperty::ReadActionUpdateTextfieldAvoidInfo(Parcel& parcel)
{
    SetTextFieldPositionY(parcel.ReadDouble());
    SetTextFieldHeight(parcel.ReadDouble());
}

void WindowSessionProperty::ReadActionUpdateWindowMask(Parcel& parcel)
{
    UnmarshallingWindowMask(parcel, this);
}

void WindowSessionProperty::ReadActionUpdateTopmost(Parcel& parcel)
{
    SetTopmost(parcel.ReadBool());
}

void WindowSessionProperty::ReadActionUpdateMainWindowTopmost(Parcel& parcel)
{
    UnmarshallingMainWindowTopmost(parcel, this);
}

void WindowSessionProperty::ReadActionUpdateSubWindowZLevel(Parcel& parcel)
{
    SetSubWindowZLevel(parcel.ReadInt32());
}

void WindowSessionProperty::ReadActionUpdateWindowModeSupportType(Parcel& parcel)
{
    SetWindowModeSupportType(parcel.ReadUint32());
}

void WindowSessionProperty::ReadActionUpdateAvoidAreaOption(Parcel& parcel)
{
    SetAvoidAreaOption(parcel.ReadUint32());
}

void WindowSessionProperty::ReadActionUpdateBackgroundAlpha(Parcel& parcel)
{
    SetBackgroundAlpha(parcel.ReadUint8());
}

void WindowSessionProperty::ReadActionUpdateExclusivelyHighlighted(Parcel& parcel)
{
    SetExclusivelyHighlighted(parcel.ReadBool());
}

void WindowSessionProperty::ReadActionUpdateFollowScreenChange(Parcel& parcel)
{
    SetFollowScreenChange(parcel.ReadBool());
}

void WindowSessionProperty::ReadActionUpdateWindowShadowEnabled(Parcel& parcel)
{
    SetWindowShadowEnabled(parcel.ReadBool());
}

void WindowSessionProperty::ReadActionUpdateAspectRatio(Parcel& parcel)
{
    SetAspectRatio(parcel.ReadFloat());
}

void WindowSessionProperty::ReadActionUpdateRotationLockChange(Parcel& parcel)
{
    SetRotationLocked(parcel.ReadBool());
}

void WindowSessionProperty::SetTransform(const Transform& trans)
{
    trans_ = trans;
}

const Transform& WindowSessionProperty::GetTransform() const
{
    return trans_;
}

void WindowSessionProperty::SetTextFieldPositionY(double textFieldPositionY)
{
    textFieldPositionY_ = textFieldPositionY;
}

void WindowSessionProperty::SetTextFieldHeight(double textFieldHeight)
{
    textFieldHeight_ = textFieldHeight;
}

double WindowSessionProperty::GetTextFieldPositionY() const
{
    return textFieldPositionY_;
}

double WindowSessionProperty::GetTextFieldHeight() const
{
    return textFieldHeight_;
}

void WindowSessionProperty::SetSessionPropertyChangeCallback(std::function<void()>&& callback)
{
    touchHotAreasChangeCallback_ = std::move(callback);
}

bool WindowSessionProperty::IsLayoutFullScreen() const
{
    return isLayoutFullScreen_;
}

void WindowSessionProperty::SetIsLayoutFullScreen(bool isLayoutFullScreen)
{
    isLayoutFullScreen_ = isLayoutFullScreen;
}

void WindowSessionProperty::SetRealParentId(int32_t realParentId)
{
    realParentId_ = realParentId;
}

int32_t WindowSessionProperty::GetRealParentId() const
{
    return realParentId_;
}

void WindowSessionProperty::SetIsUIExtFirstSubWindow(bool isUIExtFirstSubWindow)
{
    isUIExtFirstSubWindow_ = isUIExtFirstSubWindow;
}

bool WindowSessionProperty::GetIsUIExtFirstSubWindow() const
{
    return isUIExtFirstSubWindow_;
}

void WindowSessionProperty::SetIsUIExtensionAbilityProcess(bool isUIExtensionAbilityProcess)
{
    isUIExtensionAbilityProcess_ = isUIExtensionAbilityProcess;
}

bool WindowSessionProperty::GetIsUIExtensionAbilityProcess() const
{
    return isUIExtensionAbilityProcess_;
}

void WindowSessionProperty::SetIsUIExtAnySubWindow(bool isUIExtAnySubWindow)
{
    isUIExtAnySubWindow_ = isUIExtAnySubWindow;
}

bool WindowSessionProperty::GetIsUIExtAnySubWindow() const
{
    return isUIExtAnySubWindow_;
}

void WindowSessionProperty::SetUIExtensionUsage(UIExtensionUsage uiExtensionUsage)
{
    uiExtensionUsage_ = uiExtensionUsage;
}

UIExtensionUsage WindowSessionProperty::GetUIExtensionUsage() const
{
    return uiExtensionUsage_;
}

void WindowSessionProperty::SetParentWindowType(WindowType parentWindowType)
{
    parentWindowType_= parentWindowType;
}

WindowType WindowSessionProperty::GetParentWindowType() const
{
    return parentWindowType_;
}

void WindowSessionProperty::SetWindowMask(const std::shared_ptr<Media::PixelMap>& windowMask)
{
    std::lock_guard<std::mutex> lock(windowMaskMutex_);
    windowMask_ = windowMask;
}

std::shared_ptr<Media::PixelMap> WindowSessionProperty::GetWindowMask() const
{
    std::lock_guard<std::mutex> lock(windowMaskMutex_);
    return windowMask_;
}

void WindowSessionProperty::SetIsShaped(bool isShaped)
{
    isShaped_ = isShaped;
}

bool WindowSessionProperty::GetIsShaped() const
{
    return isShaped_;
}

int32_t WindowSessionProperty::GetCollaboratorType() const
{
    return collaboratorType_;
}

void WindowSessionProperty::SetCollaboratorType(int32_t collaboratorType)
{
    collaboratorType_ = collaboratorType;
}

void WindowSessionProperty::SetUserWindowLimits(const WindowLimits& windowUserLimits)
{
    userLimits_ = windowUserLimits;
}

WindowLimits WindowSessionProperty::GetUserWindowLimits() const
{
    return userLimits_;
}

void WindowSessionProperty::SetConfigWindowLimitsVP(const WindowLimits& windowConfigLimitsVP)
{
    TLOGI(WmsLogTag::WMS_LAYOUT, "id:%{public}d, windowLimits:%{public}s", GetPersistentId(),
        windowConfigLimitsVP.ToString().c_str());
    configLimitsVP_ = windowConfigLimitsVP;
}

WindowLimits WindowSessionProperty::GetConfigWindowLimitsVP() const
{
    return configLimitsVP_;
}

void WindowSessionProperty::SetLastLimitsVpr(float vpr)
{
    lastVpr_ = vpr;
}

float WindowSessionProperty::GetLastLimitsVpr() const
{
    return lastVpr_;
}

bool WindowSessionProperty::GetFullScreenStart() const
{
    return fullScreenStart_;
}

void WindowSessionProperty::SetFullScreenStart(bool fullScreenStart)
{
    fullScreenStart_ = fullScreenStart;
}

void WindowSessionProperty::SetAppInstanceKey(const std::string& appInstanceKey)
{
    appInstanceKey_ = appInstanceKey;
}

std::string WindowSessionProperty::GetAppInstanceKey() const
{
    return appInstanceKey_;
}

void WindowSessionProperty::SetAppIndex(int32_t appIndex)
{
    appIndex_ = appIndex;
}

int32_t WindowSessionProperty::GetAppIndex() const
{
    return appIndex_;
}

void WindowSessionProperty::SetIsSystemKeyboard(bool isSystemKeyboard)
{
    isSystemKeyboard_ = isSystemKeyboard;
}

bool WindowSessionProperty::IsSystemKeyboard() const
{
    return isSystemKeyboard_;
}

void WindowSessionProperty::SetKeyboardEffectOption(const KeyboardEffectOption& effectOption)
{
    std::lock_guard<std::mutex> lock(keyboardMutex_);
    keyboardEffectOption_ = effectOption;
}

KeyboardEffectOption WindowSessionProperty::GetKeyboardEffectOption() const
{
    std::lock_guard<std::mutex> lock(keyboardMutex_);
    return keyboardEffectOption_;
}

uint8_t WindowSessionProperty::GetBackgroundAlpha() const
{
    return backgroundAlpha_;
}

void WindowSessionProperty::SetBackgroundAlpha(uint8_t alpha)
{
    backgroundAlpha_ = alpha;
}

void WindowSessionProperty::SetExclusivelyHighlighted(bool isExclusivelyHighlighted)
{
    isExclusivelyHighlighted_ = isExclusivelyHighlighted;
}

bool WindowSessionProperty::GetExclusivelyHighlighted() const
{
    return isExclusivelyHighlighted_;
}

void WindowSessionProperty::SetConstrainedModal(bool isConstrained)
{
    isConstrainedModal_ = isConstrained;
}

bool WindowSessionProperty::IsConstrainedModal() const
{
    return isConstrainedModal_;
}

void WindowSessionProperty::SetApiVersion(uint32_t version)
{
    apiVersion_ = version;
}

uint32_t WindowSessionProperty::GetApiVersion() const
{
    return apiVersion_;
}

void WindowSessionProperty::SetIsFullScreenWaterfallMode(bool isFullScreenWaterfallMode)
{
    isFullScreenWaterfallMode_ = isFullScreenWaterfallMode;
}

bool WindowSessionProperty::GetIsFullScreenWaterfallMode() const
{
    return isFullScreenWaterfallMode_;
}

void WindowSessionProperty::SetIsAbilityHookOff(bool isAbilityHookOff)
{
    isAbilityHookOff_ = isAbilityHookOff;
}

bool WindowSessionProperty::GetIsAbilityHookOff() const
{
    return isAbilityHookOff_;
}

void WindowSessionProperty::SetIsAbilityHook(bool isAbilityHook)
{
    isAbilityHook_ = isAbilityHook;
}

bool WindowSessionProperty::GetIsAbilityHook() const
{
    return isAbilityHook_;
}

sptr<CompatibleModeProperty> WindowSessionProperty::GetCompatibleModeProperty() const
{
    std::lock_guard lock(compatibleModeMutex_);
    return compatibleModeProperty_;
}

void WindowSessionProperty::SetCompatibleModeProperty(const sptr<CompatibleModeProperty> property)
{
    std::lock_guard lock(compatibleModeMutex_);
    compatibleModeProperty_ = property;
}

bool WindowSessionProperty::IsAdaptToImmersive() const
{
    return compatibleModeProperty_ && compatibleModeProperty_->IsAdaptToImmersive();
}

bool WindowSessionProperty::IsAdaptToEventMapping() const
{
    return compatibleModeProperty_ && compatibleModeProperty_->IsAdaptToEventMapping();
}

bool WindowSessionProperty::IsAdaptToProportionalScale() const
{
    return compatibleModeProperty_ && compatibleModeProperty_->IsAdaptToProportionalScale();
}

bool WindowSessionProperty::IsAdaptToBackButton() const
{
    return compatibleModeProperty_ && compatibleModeProperty_->IsAdaptToBackButton();
}

bool WindowSessionProperty::IsAdaptToDragScale() const
{
    return compatibleModeProperty_ && compatibleModeProperty_->IsAdaptToDragScale();
}

bool WindowSessionProperty::IsDragResizeDisabled() const
{
    return compatibleModeProperty_ && compatibleModeProperty_->IsDragResizeDisabled();
}

bool WindowSessionProperty::IsResizeWithDpiDisabled() const
{
    return compatibleModeProperty_ && compatibleModeProperty_->IsResizeWithDpiDisabled();
}

bool WindowSessionProperty::IsFullScreenDisabled() const
{
    return compatibleModeProperty_ && compatibleModeProperty_->IsFullScreenDisabled();
}

bool WindowSessionProperty::IsSplitDisabled() const
{
    return compatibleModeProperty_ && compatibleModeProperty_->IsSplitDisabled();
}

bool WindowSessionProperty::IsWindowLimitDisabled() const
{
    return compatibleModeProperty_ && compatibleModeProperty_->IsWindowLimitDisabled();
}

bool WindowSessionProperty::IsDecorFullscreenDisabled() const
{
    return compatibleModeProperty_ && compatibleModeProperty_->IsDecorFullscreenDisabled();
}

bool WindowSessionProperty::IsFullScreenStart() const
{
    return compatibleModeProperty_ && compatibleModeProperty_->IsFullScreenStart();
}

bool WindowSessionProperty::IsSupportRotateFullScreen() const
{
    return compatibleModeProperty_ && compatibleModeProperty_->IsSupportRotateFullScreen();
}

bool WindowSessionProperty::IsAdaptToSubWindow() const
{
    return compatibleModeProperty_ && compatibleModeProperty_->IsAdaptToSubWindow();
}

bool WindowSessionProperty::IsAdaptToSimulationScale() const
{
    return compatibleModeProperty_ && compatibleModeProperty_->IsAdaptToSimulationScale();
}

bool WindowSessionProperty::IsAdaptToCompatibleDevice() const
{
    return compatibleModeProperty_ && compatibleModeProperty_->IsAdaptToCompatibleDevice();
}

RealTimeSwitchInfo WindowSessionProperty::GetRealTimeSwitchInfo() const
{
    if (!compatibleModeProperty_) {
        RealTimeSwitchInfo switchInfo;
        switchInfo.isNeedChange_ = false;
        switchInfo.showTypes_ = 0;
        return switchInfo;
    }
    return compatibleModeProperty_->GetRealTimeSwitchInfo();
}

void WindowSessionProperty::SetIsFullScreenInForceSplitMode(bool isFullScreenInForceSplitMode)
{
    isFullScreenInForceSplitMode_ = isFullScreenInForceSplitMode;
}

bool WindowSessionProperty::IsFullScreenInForceSplitMode() const
{
    return isFullScreenInForceSplitMode_;
}

void WindowSessionProperty::SetPageCompatibleMode(CompatibleStyleMode compatibleMode)
{
    pageCompatibleMode_ = compatibleMode;
}

CompatibleStyleMode WindowSessionProperty::GetPageCompatibleMode() const
{
    return pageCompatibleMode_;
}

void WindowSessionProperty::SetPcAppInpadCompatibleMode(bool enabled)
{
    isPcAppInpadCompatibleMode_ = enabled;
}

void WindowSessionProperty::SetPcAppInpadSpecificSystemBarInvisible(bool isPcAppInpadSpecificSystemBarInvisible)
{
    isPcAppInpadSpecificSystemBarInvisible_ = isPcAppInpadSpecificSystemBarInvisible;
}

void WindowSessionProperty::SetPcAppInpadOrientationLandscape(bool isPcAppInpadOrientationLandscape)
{
    isPcAppInpadOrientationLandscape_ = isPcAppInpadOrientationLandscape;
}

void WindowSessionProperty::SetMobileAppInPadLayoutFullScreen(bool isMobileAppInPadLayoutFullScreen)
{
    isMobileAppInPadLayoutFullScreen_ = isMobileAppInPadLayoutFullScreen;
}

bool WindowSessionProperty::GetPcAppInpadCompatibleMode() const
{
    return isPcAppInpadCompatibleMode_;
}

bool WindowSessionProperty::GetPcAppInpadSpecificSystemBarInvisible() const
{
    return isPcAppInpadSpecificSystemBarInvisible_;
}

bool WindowSessionProperty::GetPcAppInpadOrientationLandscape() const
{
    return isPcAppInpadOrientationLandscape_;
}

bool WindowSessionProperty::GetMobileAppInPadLayoutFullScreen() const
{
    return isMobileAppInPadLayoutFullScreen_;
}

void WindowSessionProperty::SetAncoRealBundleName(const std::string& ancoRealBundleName)
{
    ancoRealBundleName_ = ancoRealBundleName;
}

std::string WindowSessionProperty::GetAncoRealBundleName() const
{
    return ancoRealBundleName_;
}

void CompatibleModeProperty::SetIsAdaptToImmersive(bool isAdaptToImmersive)
{
    isAdaptToImmersive_ = isAdaptToImmersive;
}

bool CompatibleModeProperty::IsAdaptToImmersive() const
{
    return isAdaptToImmersive_;
}

void CompatibleModeProperty::SetIsAdaptToEventMapping(bool isAdaptToEventMapping)
{
    isAdaptToEventMapping_ = isAdaptToEventMapping;
}
        
bool CompatibleModeProperty::IsAdaptToEventMapping() const
{
    return isAdaptToEventMapping_;
}

void CompatibleModeProperty::SetIsAdaptToProportionalScale(bool isAdaptToProportionalScale)
{
    isAdaptToProportionalScale_ = isAdaptToProportionalScale;
}

bool CompatibleModeProperty::IsAdaptToProportionalScale() const
{
    return isAdaptToProportionalScale_;
}

void CompatibleModeProperty::SetIsAdaptToBackButton(bool isAdaptToBackButton)
{
    isAdaptToBackButton_ = isAdaptToBackButton;
}

bool CompatibleModeProperty::IsAdaptToBackButton() const
{
    return isAdaptToBackButton_;
}

void CompatibleModeProperty::SetIsAdaptToDragScale(bool isAdaptToDragScale)
{
    isAdaptToDragScale_ = isAdaptToDragScale;
}

bool CompatibleModeProperty::IsAdaptToDragScale() const
{
    return isAdaptToDragScale_;
}

void CompatibleModeProperty::SetDisableDragResize(bool disableDragResize)
{
    disableDragResize_ = disableDragResize;
}

bool CompatibleModeProperty::IsDragResizeDisabled() const
{
    return disableDragResize_;
}

void CompatibleModeProperty::SetDisableResizeWithDpi(bool disableResizeWithDpi)
{
    disableResizeWithDpi_ = disableResizeWithDpi;
}
        
bool CompatibleModeProperty::IsResizeWithDpiDisabled() const
{
    return disableResizeWithDpi_;
}

void CompatibleModeProperty::SetDisableFullScreen(bool disableFullScreen)
{
    disableFullScreen_ = disableFullScreen;
}

bool CompatibleModeProperty::IsFullScreenDisabled() const
{
    return disableFullScreen_;
}

void CompatibleModeProperty::SetDisableSplit(bool disableSplit)
{
    disableSplit_ = disableSplit;
}

bool CompatibleModeProperty::IsSplitDisabled() const
{
    return disableSplit_;
}

void CompatibleModeProperty::SetDisableWindowLimit(bool disableWindowLimit)
{
    disableWindowLimit_ = disableWindowLimit;
}

bool CompatibleModeProperty::IsWindowLimitDisabled() const
{
    return disableWindowLimit_;
}

void CompatibleModeProperty::SetDisableDecorFullscreen(bool disableDecorFullscreen)
{
    disableDecorFullscreen_ = disableDecorFullscreen;
}

bool CompatibleModeProperty::IsDecorFullscreenDisabled() const
{
    return disableDecorFullscreen_;
}

void CompatibleModeProperty::SetIsFullScreenStart(bool isFullScreenStart)
{
    isFullScreenStart_ = isFullScreenStart;
}

bool CompatibleModeProperty::IsFullScreenStart() const
{
    return isFullScreenStart_;
}

void CompatibleModeProperty::SetIsSupportRotateFullScreen(bool isSupportRotateFullScreen)
{
    isSupportRotateFullScreen_ = isSupportRotateFullScreen;
}

bool CompatibleModeProperty::IsSupportRotateFullScreen() const
{
    return isSupportRotateFullScreen_;
}

void CompatibleModeProperty::SetIsAdaptToSubWindow(bool isAdaptToSubWindow)
{
    isAdaptToSubWindow_ = isAdaptToSubWindow;
}

bool CompatibleModeProperty::IsAdaptToSubWindow() const
{
    return isAdaptToSubWindow_;
}

void CompatibleModeProperty::SetIsAdaptToSimulationScale(bool isAdaptToSimulationScale)
{
    isAdaptToSimulationScale_ = isAdaptToSimulationScale;
}

bool CompatibleModeProperty::IsAdaptToSimulationScale() const
{
    return isAdaptToSimulationScale_;
}

void CompatibleModeProperty::SetIsAdaptToCompatibleDevice(bool enable)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "enable=%{public}d", enable);
    isAdaptToCompatibleDevice_ = enable;
}

bool CompatibleModeProperty::IsAdaptToCompatibleDevice() const
{
    return isAdaptToCompatibleDevice_;
}

void CompatibleModeProperty::SetRealTimeSwitchInfo(const RealTimeSwitchInfo& switchInfo)
{
    realTimeSwitchInfo_.isNeedChange_ = switchInfo.isNeedChange_;
    realTimeSwitchInfo_.showTypes_ = switchInfo.showTypes_;
}

RealTimeSwitchInfo CompatibleModeProperty::GetRealTimeSwitchInfo() const
{
    return realTimeSwitchInfo_;
}

bool CompatibleModeProperty::Marshalling(Parcel& parcel) const
{
    return parcel.WriteBool(isAdaptToImmersive_) &&
        parcel.WriteBool(isAdaptToEventMapping_) &&
        parcel.WriteBool(isAdaptToProportionalScale_) &&
        parcel.WriteBool(isAdaptToBackButton_) &&
        parcel.WriteBool(isAdaptToDragScale_) &&
        parcel.WriteBool(disableDragResize_) &&
        parcel.WriteBool(disableResizeWithDpi_) &&
        parcel.WriteBool(disableFullScreen_) &&
        parcel.WriteBool(disableSplit_) &&
        parcel.WriteBool(disableWindowLimit_) &&
        parcel.WriteBool(disableDecorFullscreen_) &&
        parcel.WriteBool(isFullScreenStart_) &&
        parcel.WriteBool(isSupportRotateFullScreen_) &&
        parcel.WriteBool(isAdaptToSubWindow_) &&
        parcel.WriteBool(isAdaptToSimulationScale_) &&
        parcel.WriteBool(isAdaptToCompatibleDevice_) &&
        parcel.WriteBool(realTimeSwitchInfo_.isNeedChange_) &&
        parcel.WriteUint32(realTimeSwitchInfo_.showTypes_);
}

CompatibleModeProperty* CompatibleModeProperty::Unmarshalling(Parcel& parcel)
{
    CompatibleModeProperty* property = new(std::nothrow) CompatibleModeProperty();
    if (property == nullptr) {
        return nullptr;
    }
    property->isAdaptToImmersive_ = parcel.ReadBool();
    property->isAdaptToEventMapping_ = parcel.ReadBool();
    property->isAdaptToProportionalScale_ = parcel.ReadBool();
    property->isAdaptToBackButton_ = parcel.ReadBool();
    property->isAdaptToDragScale_ = parcel.ReadBool();
    property->disableDragResize_ = parcel.ReadBool();
    property->disableResizeWithDpi_ = parcel.ReadBool();
    property->disableFullScreen_ = parcel.ReadBool();
    property->disableSplit_ = parcel.ReadBool();
    property->disableWindowLimit_ = parcel.ReadBool();
    property->disableDecorFullscreen_ = parcel.ReadBool();
    property->isFullScreenStart_ = parcel.ReadBool();
    property->isSupportRotateFullScreen_ = parcel.ReadBool();
    property->isAdaptToSubWindow_ = parcel.ReadBool();
    property->isAdaptToSimulationScale_ = parcel.ReadBool();
    property->isAdaptToCompatibleDevice_ = parcel.ReadBool();
    property->realTimeSwitchInfo_.isNeedChange_ = parcel.ReadBool();
    property->realTimeSwitchInfo_.showTypes_ = parcel.ReadUint32();
    return property;
}

void CompatibleModeProperty::CopyFrom(const sptr<CompatibleModeProperty>& property)
{
    if (property == nullptr) {
        return;
    }
    isAdaptToImmersive_ = property->isAdaptToImmersive_;
    isAdaptToEventMapping_ = property->isAdaptToEventMapping_;
    isAdaptToProportionalScale_ = property->isAdaptToProportionalScale_;
    isAdaptToBackButton_ = property->isAdaptToBackButton_;
    disableDragResize_ = property->disableDragResize_;
    disableResizeWithDpi_ = property->disableResizeWithDpi_;
    disableFullScreen_ = property->disableFullScreen_;
    disableWindowLimit_ = property->disableWindowLimit_;
    isAdaptToSimulationScale_= property->isAdaptToSimulationScale_;
}

bool WindowSessionProperty::MarshallingWindowAnchorInfo(Parcel& parcel) const
{
    return parcel.WriteParcelable(&windowAnchorInfo_);
}

void WindowSessionProperty::UnmarshallingWindowAnchorInfo(Parcel& parcel, WindowSessionProperty* property)
{
    sptr<WindowAnchorInfo> windowAnchorInfo = parcel.ReadParcelable<WindowAnchorInfo>();
    if (windowAnchorInfo == nullptr) {
        TLOGE(WmsLogTag::WMS_SUB, "windowAnchorInfo is null");
        return;
    }
    property->SetWindowAnchorInfo(*windowAnchorInfo);
}

void WindowSessionProperty::SetMissionInfo(const MissionInfo& missionInfo)
{
    std::lock_guard<std::mutex> lock(missionInfoMutex_);
    missionInfo_ = missionInfo;
}

MissionInfo WindowSessionProperty::GetMissionInfo() const
{
    std::lock_guard<std::mutex> lock(missionInfoMutex_);
    return missionInfo_;
}

bool WindowSessionProperty::MarshallingShadowsInfo(Parcel& parcel) const
{
    return parcel.WriteParcelable(&shadowsInfo_);
}

void WindowSessionProperty::UnmarshallingShadowsInfo(Parcel& parcel, WindowSessionProperty* property)
{
    sptr<ShadowsInfo> shadowsInfo = parcel.ReadParcelable<ShadowsInfo>();
    if (shadowsInfo == nullptr) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "shadowsInfo is null");
        return;
    }
    property->SetWindowShadows(*shadowsInfo);
}

void SystemSessionConfig::ConvertSupportUIExtensionSubWindow(const std::string& itemValue)
{
    supportUIExtensionSubWindow_ = StringUtil::ConvertStringToBool(itemValue);
}

void WindowSessionProperty::SetIsShowDecorInFreeMultiWindow(bool isShow)
{
    isShowDecorInFreeMultiWindow_ = isShow;
}

bool WindowSessionProperty::GetIsShowDecorInFreeMultiWindow() const
{
    return isShowDecorInFreeMultiWindow_;
}

void WindowSessionProperty::SetAspectRatio(float ratio)
{
    aspectRatio_ = ratio;
}

float WindowSessionProperty::GetAspectRatio() const
{
    return aspectRatio_;
}

void WindowSessionProperty::SetStatusBarHeightInImmersive(int32_t statusBarHeightInImmersive)
{
    statusBarHeightInImmersive_ = statusBarHeightInImmersive;
}
 
int32_t WindowSessionProperty::GetStatusBarHeightInImmersive() const
{
    return statusBarHeightInImmersive_;
}

void WindowSessionProperty::SetPrelaunch(bool isPrelaunch)
{
    isPrelaunch_ = isPrelaunch;
}

bool WindowSessionProperty::IsPrelaunch() const
{
    return isPrelaunch_;
}

void WindowSessionProperty::SetFrameNum(int32_t frameNum)
{
    frameNum_ = frameNum;
}

int32_t WindowSessionProperty::GetFrameNum() const
{
    return frameNum_;
}
} // namespace Rosen
} // namespace OHOS