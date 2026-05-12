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

#include "window_option.h"

#include "window_helper.h"
#include "wm_common.h"

namespace OHOS {
namespace Rosen {
WindowOption::WindowOption(): windowTag_(WindowTag::SYSTEM_WINDOW)
{
    AddWindowFlag(WindowFlag::WINDOW_FLAG_NEED_AVOID);
}

void WindowOption::SetWindowMode(WindowMode mode)
{
    if (!WindowHelper::IsValidWindowMode(mode)) {
        return;
    }
    mode_ = mode;
}

void WindowOption::SetWindowRect(const Rect& rect)
{
    windowRect_ = rect;
}

void WindowOption::SetWindowType(WindowType type)
{
    type_ = type;
}

void WindowOption::SetParentId(uint32_t parentId)
{
    parentId_ = parentId;
}

void WindowOption::SetDisplayId(DisplayId displayId)
{
    displayId_ = displayId;
}

void WindowOption::SetFocusable(bool isFocusable)
{
    focusable_ = isFocusable;
}

void WindowOption::SetTouchable(bool isTouchable)
{
    touchable_ = isTouchable;
}

void WindowOption::SetWindowName(const std::string& windowName)
{
    windowName_ = windowName;
}

void WindowOption::SetWindowFlags(uint32_t flags)
{
    flags_ = flags;
}

void WindowOption::RemoveWindowFlag(WindowFlag flag)
{
    flags_ &= ~(static_cast<uint32_t>(flag));
}

void WindowOption::AddWindowFlag(WindowFlag flag)
{
    flags_ |= static_cast<uint32_t>(flag);
}

void WindowOption::SetWindowTag(WindowTag windowTag)
{
    windowTag_ = windowTag;
}

void WindowOption::SetSystemBarProperty(WindowType type, const SystemBarProperty& property)
{
    if (type == WindowType::WINDOW_TYPE_STATUS_BAR || type == WindowType::WINDOW_TYPE_NAVIGATION_BAR) {
        sysBarPropMap_[type] = property;
    }
}

void WindowOption::SetWindowSessionType(WindowSessionType sessionType)
{
    sessionType_ = sessionType;
}

void WindowOption::SetHitOffset(int32_t x, int32_t y)
{
    hitOffset_.x = x;
    hitOffset_.y = y;
}

bool WindowOption::IsTurnScreenOn() const
{
    return turnScreenOn_;
}

void WindowOption::SetTurnScreenOn(bool turnScreenOn)
{
    turnScreenOn_ = turnScreenOn;
}

bool WindowOption::IsKeepScreenOn() const
{
    return keepScreenOn_;
}

void WindowOption::SetKeepScreenOn(bool keepScreenOn)
{
    keepScreenOn_ = keepScreenOn;
}

void WindowOption::ChangeCallingWindowId(uint32_t windowId)
{
    callingWindow_ = windowId;
}

void WindowOption::SetRequestedOrientation(Orientation orientation)
{
    requestedOrientation_ = orientation;
}

void WindowOption::SetBrightness(float brightness)
{
    brightness_ = brightness;
}

void WindowOption::SetMainHandlerAvailable(bool isMainHandlerAvailable)
{
    isMainHandlerAvailable_ = isMainHandlerAvailable;
}

void WindowOption::SetSubWindowDecorEnable(bool subWindowDecorEnable)
{
    subWindowDecorEnable_ = subWindowDecorEnable;
}

void WindowOption::SetSubWindowTitle(const std::string& subWindowTitle)
{
    subWindowTitle_ = subWindowTitle;
}

void WindowOption::SetSubWindowMaximizeSupported(bool maximizeSupported)
{
    subWindowMaximizeSupported_ = maximizeSupported;
}

void WindowOption::SetOnlySupportSceneBoard(bool onlySupportSceneBoard)
{
    onlySupportSceneBoard_ = onlySupportSceneBoard;
}

void WindowOption::SetWindowTopmost(bool isTopmost)
{
    isTopmost_ = isTopmost;
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

WindowMode WindowOption::GetWindowMode() const
{
    return mode_;
}

WindowType WindowOption::GetWindowType() const
{
    return type_;
}

Rect WindowOption::GetWindowRect() const
{
    return windowRect_;
}

WindowTag WindowOption::GetWindowTag() const
{
    return windowTag_;
}

uint32_t WindowOption::GetParentId() const
{
    return parentId_;
}

DisplayId WindowOption::GetDisplayId() const
{
    return displayId_;
}

uint32_t WindowOption::GetWindowFlags() const
{
    return flags_;
}

bool WindowOption::GetTouchable() const
{
    return touchable_;
}

bool WindowOption::GetFocusable() const
{
    return focusable_;
}

const PointInfo& WindowOption::GetHitOffset() const
{
    return hitOffset_;
}

const std::string& WindowOption::GetWindowName() const
{
    return windowName_;
}

WindowSessionType WindowOption::GetWindowSessionType() const
{
    return sessionType_;
}

bool WindowOption::GetMainHandlerAvailable() const
{
    return isMainHandlerAvailable_;
}

const std::unordered_map<WindowType, SystemBarProperty>& WindowOption::GetSystemBarProperty() const
{
    return sysBarPropMap_;
}

bool WindowOption::GetOnlySupportSceneBoard() const
{
    return onlySupportSceneBoard_;
}

float WindowOption::GetBrightness() const
{
    return brightness_;
}

Orientation WindowOption::GetRequestedOrientation() const
{
    return requestedOrientation_;
}

uint32_t WindowOption::GetCallingWindow() const
{
    return callingWindow_;
}

bool WindowOption::GetSubWindowDecorEnable() const
{
    return subWindowDecorEnable_;
}

std::string WindowOption::GetSubWindowTitle() const
{
    return subWindowTitle_;
}

bool WindowOption::GetWindowTopmost() const
{
    return isTopmost_;
}

bool WindowOption::GetSubWindowMaximizeSupported() const
{
    return subWindowMaximizeSupported_;
}

void WindowOption::SetSubWindowOutlineEnabled(bool outlineEnabled)
{
    subWindowOutlineEnabled_ = outlineEnabled;
}

bool WindowOption::IsSubWindowOutlineEnabled() const
{
    return subWindowOutlineEnabled_;
}

void WindowOption::SetZLevelAboveParentLoosened(bool zLevelAboveParentLoosened)
{
    zLevelAboveParentLoosened_ = zLevelAboveParentLoosened;
}
 
bool WindowOption::IsSubWindowZLevelAboveParentLoosened() const
{
    return zLevelAboveParentLoosened_;
}
} // namespace Rosen
} // namespace OHOS

