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

#include "window_option.h"
#include "window_helper.h"
#include "wm_common.h"

namespace OHOS {
namespace Rosen {
WindowOption::WindowOption(): windowTag_(WindowTag::SYSTEM_WINDOW)
{
}

void WindowOption::SetWindowRect(const Rect& rect)
{
    windowRect_ = rect;
}

void WindowOption::SetWindowType(WindowType type)
{
    type_ = type;
}

void WindowOption::SetWindowMode(WindowMode mode)
{
    if (!WindowHelper::IsValidWindowMode(mode)) {
        return;
    }
    mode_ = mode;
}

void WindowOption::SetFocusable(bool isFocusable)
{
    focusable_ = isFocusable;
}

void WindowOption::SetTouchable(bool isTouchable)
{
    touchable_ = isTouchable;
}

void WindowOption::SetDisplayId(DisplayId displayId)
{
    displayId_ = displayId;
}

void WindowOption::SetParentId(uint32_t parentId)
{
    parentId_ = parentId;
}

void WindowOption::SetWindowName(const std::string& windowName)
{
    windowName_ = windowName;
}

void WindowOption::AddWindowFlag(WindowFlag flag)
{
    flags_ |= static_cast<uint32_t>(flag);
}

void WindowOption::RemoveWindowFlag(WindowFlag flag)
{
    flags_ &= ~(static_cast<uint32_t>(flag));
}

void WindowOption::SetWindowFlags(uint32_t flags)
{
    flags_ = flags;
}

void WindowOption::SetSystemBarProperty(WindowType type, const SystemBarProperty& property)
{
    if (type == WindowType::WINDOW_TYPE_STATUS_BAR || type == WindowType::WINDOW_TYPE_NAVIGATION_BAR) {
        sysBarPropMap_[type] = property;
    }
}

Rect WindowOption::GetWindowRect() const
{
    return windowRect_;
}

WindowType WindowOption::GetWindowType() const
{
    return type_;
}

WindowMode WindowOption::GetWindowMode() const
{
    return mode_;
}

bool WindowOption::GetFocusable() const
{
    return focusable_;
}

bool WindowOption::GetTouchable() const
{
    return touchable_;
}

DisplayId WindowOption::GetDisplayId() const
{
    return displayId_;
}

uint32_t WindowOption::GetParentId() const
{
    return parentId_;
}

const std::string& WindowOption::GetWindowName() const
{
    return windowName_;
}

uint32_t WindowOption::GetWindowFlags() const
{
    return flags_;
}

void WindowOption::SetHitOffset(int32_t x, int32_t y)
{
    hitOffset_.x = x;
    hitOffset_.y = y;
}

void WindowOption::SetWindowTag(WindowTag windowTag)
{
    windowTag_ = windowTag;
}

WindowTag WindowOption::GetWindowTag() const
{
    return windowTag_;
}

void WindowOption::SetWindowSessionType(WindowSessionType sessionType)
{
    sessionType_ = sessionType;
}

WindowSessionType WindowOption::GetWindowSessionType() const
{
    return sessionType_;
}

void WindowOption::SetMainHandlerAvailable(bool isMainHandlerAvailable)
{
    isMainHandlerAvailable_ = isMainHandlerAvailable;
}

bool WindowOption::GetMainHandlerAvailable() const
{
    return isMainHandlerAvailable_;
}

const PointInfo& WindowOption::GetHitOffset() const
{
    return hitOffset_;
}

const std::unordered_map<WindowType, SystemBarProperty>& WindowOption::GetSystemBarProperty() const
{
    return sysBarPropMap_;
}

void WindowOption::SetKeepScreenOn(bool keepScreenOn)
{
    keepScreenOn_ = keepScreenOn;
}

bool WindowOption::IsKeepScreenOn() const
{
    return keepScreenOn_;
}

void WindowOption::SetViewKeepScreenOn(bool keepScreenOn)
{
    viewKeepScreenOn_ = keepScreenOn;
}

bool WindowOption::IsViewKeepScreenOn() const
{
    return viewKeepScreenOn_;
}

void WindowOption::SetTurnScreenOn(bool turnScreenOn)
{
    turnScreenOn_ = turnScreenOn;
}

bool WindowOption::IsTurnScreenOn() const
{
    return turnScreenOn_;
}

void WindowOption::SetBrightness(float brightness)
{
    brightness_ = brightness;
}

float WindowOption::GetBrightness() const
{
    return brightness_;
}

void WindowOption::ChangeCallingWindowId(uint32_t windowId)
{
    callingWindow_ = windowId;
}

uint32_t WindowOption::GetCallingWindow() const
{
    return callingWindow_;
}

Orientation WindowOption::GetRequestedOrientation() const
{
    return requestedOrientation_;
}

void WindowOption::SetRequestedOrientation(Orientation orientation)
{
    requestedOrientation_ = orientation;
}

void WindowOption::SetBundleName(const std::string bundleName)
{
    bundleName_ = bundleName;
}

const std::string WindowOption::GetBundleName() const
{
    return bundleName_;
}

void WindowOption::SetSubWindowMaximizeSupported(bool maximizeSupported)
{
    subWindowMaximizeSupported_ = maximizeSupported;
}

bool WindowOption::GetSubWindowMaximizeSupported() const
{
    return subWindowMaximizeSupported_;
}

void WindowOption::SetSubWindowTitle(const std::string& subWindowTitle)
{
    subWindowTitle_ = subWindowTitle;
}

std::string WindowOption::GetSubWindowTitle() const
{
    return subWindowTitle_;
}

void WindowOption::SetSubWindowDecorEnable(bool subWindowDecorEnable)
{
    subWindowDecorEnable_ = subWindowDecorEnable;
}

bool WindowOption::GetSubWindowDecorEnable() const
{
    return subWindowDecorEnable_;
}

void WindowOption::SetOnlySupportSceneBoard(bool onlySupportSceneBoard)
{
    onlySupportSceneBoard_ = onlySupportSceneBoard;
}

bool WindowOption::GetOnlySupportSceneBoard() const
{
    return onlySupportSceneBoard_;
}

void WindowOption::SetRealParentId(int32_t realParentId)
{
    realParentId_ = realParentId;
}

int32_t WindowOption::GetRealParentId() const
{
    return realParentId_;
}

void WindowOption::SetParentWindowType(WindowType parentWindowType)
{
    parentWindowType_ = parentWindowType;
}

WindowType WindowOption::GetParentWindowType() const
{
    return parentWindowType_;
}

void WindowOption::SetIsUIExtFirstSubWindow(bool isUIExtFirstSubWindow)
{
    isUIExtFirstSubWindow_ = isUIExtFirstSubWindow;
}

bool WindowOption::GetIsUIExtFirstSubWindow() const
{
    return isUIExtFirstSubWindow_;
}

void WindowOption::SetUIExtensionUsage(uint32_t uiExtensionUsage)
{
    if (uiExtensionUsage < static_cast<uint32_t>(UIExtensionUsage::UIEXTENSION_USAGE_END)) {
        uiExtensionUsage_ = uiExtensionUsage;
    } else {
        uiExtensionUsage_ = static_cast<uint32_t>(UIExtensionUsage::EMBEDDED);
    }
}

uint32_t WindowOption::GetUIExtensionUsage() const
{
    return uiExtensionUsage_;
}

void WindowOption::SetDialogDecorEnable(bool decorEnable)
{
    dialogDecorEnable_ = decorEnable;
}

void WindowOption::SetIsUIExtAnySubWindow(bool isUIExtAnySubWindow)
{
    isUIExtAnySubWindow_ = isUIExtAnySubWindow;
}

bool WindowOption::GetIsUIExtAnySubWindow() const
{
    return isUIExtAnySubWindow_;
}

bool WindowOption::GetDialogDecorEnable() const
{
    return dialogDecorEnable_;
}

void WindowOption::SetDialogTitle(const std::string& dialogTitle)
{
    dialogTitle_ = dialogTitle;
}

std::string WindowOption::GetDialogTitle() const
{
    return dialogTitle_;
}

void WindowOption::SetWindowTopmost(bool isTopmost)
{
    isTopmost_ = isTopmost;
}

bool WindowOption::GetWindowTopmost() const
{
    return isTopmost_;
}

void WindowOption::SetSubWindowZLevel(int32_t zLevel)
{
    zLevel_ = zLevel;
}

int32_t WindowOption::GetSubWindowZLevel() const
{
    return zLevel_;
}

void WindowOption::SetZIndex(int32_t zIndex)
{
    zIndex_ = zIndex;
}

int32_t WindowOption::GetZIndex() const
{
    return zIndex_;
}

void WindowOption::SetIsSystemKeyboard(bool isSystemKeyboard)
{
    isSystemKeyboard_ = isSystemKeyboard;
}

bool WindowOption::IsSystemKeyboard() const
{
    return isSystemKeyboard_;
}

void WindowOption::SetDensity(float density)
{
    density_ = density;
}

float WindowOption::GetDensity() const
{
    return density_;
}

void WindowOption::SetIsDensityFollowHost(bool isDensityFollowHost)
{
    isDensityFollowHost_ = isDensityFollowHost;
}

bool WindowOption::GetIsDensityFollowHost() const
{
    return isDensityFollowHost_;
}

void WindowOption::SetDefaultDensityEnabled(bool defaultDensityEnabled)
{
    defaultDensityEnabled_ = defaultDensityEnabled;
}

bool WindowOption::IsDefaultDensityEnabled() const
{
    return defaultDensityEnabled_;
}

void WindowOption::SetConstrainedModal(bool isConstrainedModal)
{
    isConstrainedModal_ = (uiExtensionUsage_ == static_cast<uint32_t>(UIExtensionUsage::MODAL)) && isConstrainedModal;
}

bool WindowOption::IsConstrainedModal() const
{
    return isConstrainedModal_;
}

void WindowOption::SetSubWindowOutlineEnabled(bool outlineEnabled)
{
    subWindowOutlineEnabled_ = outlineEnabled;
}

bool WindowOption::IsSubWindowOutlineEnabled() const
{
    return subWindowOutlineEnabled_;
}

int64_t WindowOption::GetStartModalExtensionTimeStamp() const
{
    return startModalExtensionTimeStamp_;
}

void WindowOption::SetStartModalExtensionTimeStamp(int64_t timeStamp)
{
    startModalExtensionTimeStamp_ = timeStamp;
}

void WindowOption::SetFollowCreatorLifecycle(bool followCreatorLifecycle)
{
    followCreatorLifecycle_ = followCreatorLifecycle;
}

bool WindowOption::IsFollowCreatorLifecycle() const
{
    return followCreatorLifecycle_;
}
} // namespace Rosen
} // namespace OHOS

