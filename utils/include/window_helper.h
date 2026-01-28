/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_WM_INCLUDE_WM_HELPER_H
#define OHOS_WM_INCLUDE_WM_HELPER_H

#include <unistd.h>
#include <vector>
#include "ability_info.h"
#include "window_transition_info.h"
#include "wm_common.h"
#include "wm_common_inner.h"
#include "wm_math.h"

namespace OHOS {
namespace Rosen {
class WindowHelper {
public:
    static inline bool IsMainWindow(WindowType type)
    {
        return (type >= WindowType::APP_MAIN_WINDOW_BASE && type < WindowType::APP_MAIN_WINDOW_END);
    }

    static inline bool IsMainWindowAndNotShown(WindowType type, WindowState state)
    {
        return (IsMainWindow(type) && state != WindowState::STATE_SHOWN);
    }

    static inline bool IsModalMainWindow(WindowType type, uint32_t windowFlags)
    {
        return IsMainWindow(type) && (windowFlags & static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_IS_MODAL));
    }

    static inline bool IsSubWindow(WindowType type)
    {
        return (type >= WindowType::APP_SUB_WINDOW_BASE && type < WindowType::APP_SUB_WINDOW_END);
    }

    static inline bool IsNormalSubWindow(WindowType type, uint32_t windowFlags)
    {
        const uint32_t mask = static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_IS_MODAL) |
            static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_IS_TOAST) |
            static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_IS_TEXT_MENU);
        return ((windowFlags & mask) == 0 && IsSubWindow(type));
    }

    static inline bool IsModalSubWindow(WindowType type, uint32_t windowFlags)
    {
        return IsSubWindow(type) && (windowFlags & static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_IS_MODAL));
    }

    static inline bool IsApplicationModalSubWindow(WindowType type, uint32_t windowFlags)
    {
        return IsModalSubWindow(type, windowFlags) &&
            (windowFlags & static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_IS_APPLICATION_MODAL));
    }

    static inline bool IsToastSubWindow(WindowType type, uint32_t windowFlags)
    {
        return IsSubWindow(type) && (windowFlags & static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_IS_TOAST));
    }

    static inline bool IsModalWindow(uint32_t windowFlags)
    {
        return (windowFlags & static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_IS_APPLICATION_MODAL)) ||
            (windowFlags & static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_IS_MODAL));
    }

    static inline bool IsTextMenuSubWindow(WindowType type, uint32_t windowFlags)
    {
        return IsSubWindow(type) && (windowFlags & static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_IS_TEXT_MENU));
    }

    static inline bool IsDialogWindow(WindowType type)
    {
        return type == WindowType::WINDOW_TYPE_DIALOG;
    }

    static inline bool IsAppWindow(WindowType type)
    {
        return (IsMainWindow(type) || IsSubWindow(type));
    }

    static inline bool IsAppFloatingWindow(WindowType type)
    {
        return (type == WindowType::WINDOW_TYPE_FLOAT) || (type == WindowType::WINDOW_TYPE_FLOAT_CAMERA);
    }

    static inline bool IsFloatOrSubWindow(WindowType type)
    {
        return type == WindowType::WINDOW_TYPE_FLOAT || IsSubWindow(type);
    }

    static inline bool IsPipWindow(WindowType type)
    {
        return (type == WindowType::WINDOW_TYPE_PIP);
    }

    static inline bool IsFbWindow(WindowType type)
    {
        return (type == WindowType::WINDOW_TYPE_FB);
    }

    static inline bool IsBelowSystemWindow(WindowType type)
    {
        return (type >= WindowType::BELOW_APP_SYSTEM_WINDOW_BASE && type < WindowType::BELOW_APP_SYSTEM_WINDOW_END);
    }

    static inline bool IsAboveSystemWindow(WindowType type)
    {
        return (type >= WindowType::ABOVE_APP_SYSTEM_WINDOW_BASE && type < WindowType::ABOVE_APP_SYSTEM_WINDOW_END);
    }

    static inline bool IsSystemSubWindow(WindowType type)
    {
        return (type >= WindowType::SYSTEM_SUB_WINDOW_BASE && type < WindowType::SYSTEM_SUB_WINDOW_END);
    }

    static inline bool IsSystemMainWindow(WindowType type)
    {
        return IsBelowSystemWindow(type) || IsAboveSystemWindow(type);
    }

    static inline bool IsSystemWindow(WindowType type)
    {
        return (IsBelowSystemWindow(type) || IsAboveSystemWindow(type) || IsSystemSubWindow(type));
    }

    static inline bool IsSystemWindowButNotDialog(WindowType type)
    {
        return IsSystemWindow(type) && !IsDialogWindow(type);
    }

    static inline bool IsUIExtensionWindow(WindowType type)
    {
        return (type == WindowType::WINDOW_TYPE_UI_EXTENSION);
    }

    static inline bool IsAppComponentWindow(WindowType type)
    {
        return (type == WindowType::WINDOW_TYPE_APP_COMPONENT);
    }

    static inline bool IsMainFloatingWindow(WindowType type, WindowMode mode)
    {
        return ((IsMainWindow(type)) && (mode == WindowMode::WINDOW_MODE_FLOATING));
    }

    static inline bool IsMainFullScreenWindow(WindowType type, WindowMode mode)
    {
        return ((IsMainWindow(type)) && (mode == WindowMode::WINDOW_MODE_FULLSCREEN));
    }

    static inline bool IsFloatingWindow(WindowMode mode)
    {
        return mode == WindowMode::WINDOW_MODE_FLOATING;
    }

    static inline bool IsSystemBarWindow(WindowType type)
    {
        return (type == WindowType::WINDOW_TYPE_STATUS_BAR || type == WindowType::WINDOW_TYPE_NAVIGATION_BAR);
    }

    static inline bool IsOverlayWindow(WindowType type)
    {
        return (type == WindowType::WINDOW_TYPE_STATUS_BAR ||
            type == WindowType::WINDOW_TYPE_NAVIGATION_BAR ||
            type == WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    }

    static inline bool IsRotatableWindow(WindowType type, WindowMode mode)
    {
        return WindowHelper::IsMainFullScreenWindow(type, mode) || type == WindowType::WINDOW_TYPE_KEYGUARD ||
            type == WindowType::WINDOW_TYPE_DESKTOP ||
            ((type == WindowType::WINDOW_TYPE_LAUNCHER_RECENT) && (mode == WindowMode::WINDOW_MODE_FULLSCREEN));
    }

    static inline bool IsInputWindow(WindowType type)
    {
        return (type == WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT ||
                type == WindowType::WINDOW_TYPE_INPUT_METHOD_STATUS_BAR);
    }

    static inline bool IsKeyboardWindow(WindowType type)
    {
        return type == WindowType::WINDOW_TYPE_KEYBOARD_PANEL || type == WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT;
    }

    static inline bool IsSystemOrSubWindow(WindowType type)
    {
        return IsSubWindow(type) || IsSystemWindow(type);
    }

    static inline bool IsNeedWaitAttachStateWindow(WindowType type)
    {
        return !IsKeyboardWindow(type) && IsSystemOrSubWindow(type);
    }

    static inline bool IsDynamicWindow(WindowType type)
    {
        return type == WindowType::WINDOW_TYPE_DYNAMIC;
    }

    static inline bool IsWindowInApp(WindowType type)
    {
        return (type == WindowType::WINDOW_TYPE_FLOAT || type == WindowType::WINDOW_TYPE_DIALOG ||
                type == WindowType::WINDOW_TYPE_PIP || type == WindowType::WINDOW_TYPE_FB);
    }

    static inline bool IsFullScreenWindow(WindowMode mode)
    {
        return mode == WindowMode::WINDOW_MODE_FULLSCREEN;
    }

    static inline bool IsSplitWindowMode(WindowMode mode)
    {
        return mode == WindowMode::WINDOW_MODE_SPLIT_PRIMARY || mode == WindowMode::WINDOW_MODE_SPLIT_SECONDARY;
    }

    static inline bool IsPipWindowMode(WindowMode mode)
    {
        return mode == WindowMode::WINDOW_MODE_PIP;
    }

    static inline bool IsAppFullOrSplitWindow(WindowType type, WindowMode mode)
    {
        if (!IsAppWindow(type)) {
            return false;
        }
        return IsFullScreenWindow(mode) || IsSplitWindowMode(mode);
    }

    static inline bool IsValidWindowMode(WindowMode mode)
    {
        return mode == WindowMode::WINDOW_MODE_FULLSCREEN || mode == WindowMode::WINDOW_MODE_SPLIT_PRIMARY ||
            mode == WindowMode::WINDOW_MODE_SPLIT_SECONDARY || mode == WindowMode::WINDOW_MODE_FLOATING ||
            mode == WindowMode::WINDOW_MODE_PIP || mode == WindowMode::WINDOW_MODE_FB;
    }

    static inline bool IsEmptyRect(const Rect& r)
    {
        return (r.posX_ == 0 && r.posY_ == 0 && r.width_ == 0 && r.height_ == 0);
    }

    static inline bool IsLandscapeRect(const Rect& r)
    {
        return r.width_ > r.height_;
    }

    static inline bool IsShowWhenLocked(uint32_t flags)
    {
        return flags & static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_SHOW_WHEN_LOCKED);
    }

    static Rect GetOverlap(const Rect& rect1, const Rect& rect2, const int offsetX, const int offsetY)
    {
        int32_t x_begin = std::max(rect1.posX_, rect2.posX_);
        int32_t x_end = std::min(rect1.posX_ + static_cast<int32_t>(rect1.width_),
            rect2.posX_ + static_cast<int32_t>(rect2.width_));
        int32_t y_begin = std::max(rect1.posY_, rect2.posY_);
        int32_t y_end = std::min(rect1.posY_ + static_cast<int32_t>(rect1.height_),
            rect2.posY_ + static_cast<int32_t>(rect2.height_));
        if (y_begin >= y_end || x_begin >= x_end) {
            return { 0, 0, 0, 0 };
        }
        return { x_begin - offsetX, y_begin - offsetY,
            static_cast<uint32_t>(x_end - x_begin), static_cast<uint32_t>(y_end - y_begin) };
    }

    static bool IsWindowModeSupported(uint32_t windowModeSupportType, WindowMode mode)
    {
        switch (mode) {
            case WindowMode::WINDOW_MODE_FULLSCREEN:
                return WindowModeSupport::WINDOW_MODE_SUPPORT_FULLSCREEN & windowModeSupportType;
            case WindowMode::WINDOW_MODE_FLOATING:
                return WindowModeSupport::WINDOW_MODE_SUPPORT_FLOATING & windowModeSupportType;
            case WindowMode::WINDOW_MODE_SPLIT_PRIMARY:
                return WindowModeSupport::WINDOW_MODE_SUPPORT_SPLIT_PRIMARY & windowModeSupportType;
            case WindowMode::WINDOW_MODE_SPLIT_SECONDARY:
                return WindowModeSupport::WINDOW_MODE_SUPPORT_SPLIT_SECONDARY & windowModeSupportType;
            case WindowMode::WINDOW_MODE_PIP:
                return WindowModeSupport::WINDOW_MODE_SUPPORT_PIP & windowModeSupportType;
            case WindowMode::WINDOW_MODE_FB:
                return WindowModeSupport::WINDOW_MODE_SUPPORT_FB & windowModeSupportType;
            case WindowMode::WINDOW_MODE_UNDEFINED:
                return false;
            default:
                return true;
        }
    }

    static WindowMode GetWindowModeFromWindowModeSupportType(uint32_t windowModeSupportType)
    {
        // get the binary number consists of the last 1 and 0 behind it
        uint32_t windowModeSupport = windowModeSupportType & (~windowModeSupportType + 1);

        switch (windowModeSupport) {
            case WindowModeSupport::WINDOW_MODE_SUPPORT_FULLSCREEN:
                return WindowMode::WINDOW_MODE_FULLSCREEN;
            case WindowModeSupport::WINDOW_MODE_SUPPORT_FLOATING:
                return WindowMode::WINDOW_MODE_FLOATING;
            case WindowModeSupport::WINDOW_MODE_SUPPORT_SPLIT_PRIMARY:
                return WindowMode::WINDOW_MODE_SPLIT_PRIMARY;
            case WindowModeSupport::WINDOW_MODE_SUPPORT_SPLIT_SECONDARY:
                return WindowMode::WINDOW_MODE_SPLIT_SECONDARY;
            case WindowModeSupport::WINDOW_MODE_SUPPORT_PIP:
                return WindowMode::WINDOW_MODE_PIP;
            case WindowModeSupport::WINDOW_MODE_SUPPORT_FB:
                return WindowMode::WINDOW_MODE_FB;
            default:
                return WindowMode::WINDOW_MODE_UNDEFINED;
        }
    }

    static uint32_t ConvertSupportModesToSupportType(const std::vector<AppExecFwk::SupportWindowMode>& supportModes)
    {
        uint32_t windowModeSupportType = 0;
        for (auto& mode : supportModes) {
            if (mode == AppExecFwk::SupportWindowMode::FULLSCREEN) {
                windowModeSupportType |= WindowModeSupport::WINDOW_MODE_SUPPORT_FULLSCREEN;
            } else if (mode == AppExecFwk::SupportWindowMode::SPLIT) {
                windowModeSupportType |= (WindowModeSupport::WINDOW_MODE_SUPPORT_SPLIT_PRIMARY |
                                    WindowModeSupport::WINDOW_MODE_SUPPORT_SPLIT_SECONDARY);
            } else if (mode == AppExecFwk::SupportWindowMode::FLOATING) {
                windowModeSupportType |= WindowModeSupport::WINDOW_MODE_SUPPORT_FLOATING;
            }
        }
        return windowModeSupportType;
    }

    static std::vector<AppExecFwk::SupportWindowMode> ConvertSupportTypeToSupportModes(uint32_t windowModeSupportType)
    {
        std::vector<AppExecFwk::SupportWindowMode> supportModes;
        if ((windowModeSupportType & WindowModeSupport::WINDOW_MODE_SUPPORT_FULLSCREEN) != 0) {
            supportModes.push_back(AppExecFwk::SupportWindowMode::FULLSCREEN);
        }
        if ((windowModeSupportType & (WindowModeSupport::WINDOW_MODE_SUPPORT_SPLIT_PRIMARY |
            WindowModeSupport::WINDOW_MODE_SUPPORT_SPLIT_SECONDARY)) != 0) {
            supportModes.push_back(AppExecFwk::SupportWindowMode::SPLIT);
        }
        if ((windowModeSupportType & WindowModeSupport::WINDOW_MODE_SUPPORT_FLOATING) != 0) {
            supportModes.push_back(AppExecFwk::SupportWindowMode::FLOATING);
        }
        return supportModes;
    }

    static bool IsPointInTargetRect(int32_t pointPosX, int32_t pointPosY, const Rect& targetRect)
    {
        if ((pointPosX > targetRect.posX_) &&
            (pointPosX < (targetRect.posX_ + static_cast<int32_t>(targetRect.width_)) - 1) &&
            (pointPosY > targetRect.posY_) &&
            (pointPosY < (targetRect.posY_ + static_cast<int32_t>(targetRect.height_)) - 1)) {
            return true;
        }
        return false;
    }

    static bool IsPointInTargetRectWithBound(int32_t pointPosX, int32_t pointPosY, const Rect& targetRect)
    {
        if ((pointPosX >= targetRect.posX_) &&
            (pointPosX < (targetRect.posX_ + static_cast<int32_t>(targetRect.width_))) &&
            (pointPosY >= targetRect.posY_) &&
            (pointPosY < (targetRect.posY_ + static_cast<int32_t>(targetRect.height_)))) {
            return true;
        }
        return false;
    }

    static bool IsPointInWindowExceptCorner(int32_t pointPosX, int32_t pointPosY, const Rect& rectExceptCorner)
    {
        if ((pointPosX > rectExceptCorner.posX_ &&
            pointPosX < (rectExceptCorner.posX_ + static_cast<int32_t>(rectExceptCorner.width_)) - 1) ||
            (pointPosY > rectExceptCorner.posY_ &&
            pointPosY < (rectExceptCorner.posY_ + static_cast<int32_t>(rectExceptCorner.height_)) - 1)) {
            return true;
        }
        return false;
    }

    static inline bool IsSwitchCascadeReason(WindowUpdateReason reason)
    {
        return (reason >= WindowUpdateReason::NEED_SWITCH_CASCADE_BASE) &&
            (reason < WindowUpdateReason::NEED_SWITCH_CASCADE_END);
    }

    static AvoidPosType GetAvoidPosType(const Rect& rect, const Rect& displayRect)
    {
        if (rect.width_ ==  displayRect.width_) {
            if (rect.posY_ == displayRect.posY_) {
                return AvoidPosType::AVOID_POS_TOP;
            } else {
                return AvoidPosType::AVOID_POS_BOTTOM;
            }
        } else if (rect.height_ ==  displayRect.height_) {
            if (rect.posX_ == displayRect.posX_) {
                return AvoidPosType::AVOID_POS_LEFT;
            } else {
                return AvoidPosType::AVOID_POS_RIGHT;
            }
        }

        return AvoidPosType::AVOID_POS_UNKNOWN;
    }

    static inline bool IsNumber(std::string str)
    {
        if (str.size() == 0) {
            return false;
        }
        for (int32_t i = 0; i < static_cast<int32_t>(str.size()); i++) {
            if (str.at(i) < '0' || str.at(i) > '9') {
                return false;
            }
        }
        return true;
    }

    static bool IsFloatingNumber(std::string str, bool allowNeg = false)
    {
        if (str.size() == 0) {
            return false;
        }

        int32_t i = 0;
        if (allowNeg && str.at(i) == '-') {
            i++;
        }

        for (; i < static_cast<int32_t>(str.size()); i++) {
            if ((str.at(i) < '0' || str.at(i) > '9') &&
                (str.at(i) != '.' || std::count(str.begin(), str.end(), '.') > 1)) {
                return false;
            }
        }
        return true;
    }

    static std::vector<std::string> Split(std::string str, std::string pattern)
    {
        int32_t position;
        std::vector<std::string> result;
        str += pattern;
        int32_t length = static_cast<int32_t>(str.size());
        for (int32_t i = 0; i < length; i++) {
            position = static_cast<int32_t>(str.find(pattern, i));
            if (position < length) {
                std::string tmp = str.substr(i, position - i);
                result.push_back(tmp);
                i = position + static_cast<int32_t>(pattern.size()) - 1;
            }
        }
        return result;
    }

    static PointInfo CalculateOriginPosition(const Rect& rOrigin, const Rect& rActial, const PointInfo& pos)
    {
        PointInfo ret = pos;
        ret.x += rActial.posX_ - pos.x;
        ret.y += rActial.posY_ - pos.y;
        ret.x += rOrigin.posX_ - rActial.posX_;
        ret.y += rOrigin.posY_ - rActial.posY_;
        if (rActial.width_ && rActial.height_) {
            ret.x += (pos.x - rActial.posX_) * rOrigin.width_ / rActial.width_;
            ret.y += (pos.y - rActial.posY_) * rOrigin.height_ / rActial.height_;
        }
        return ret;
    }

    // Transform a point at screen to its oringin position in 3D world and project to xy plane
    static PointInfo CalculateOriginPosition(const TransformHelper::Matrix4& transformMat, const PointInfo& pointPos)
    {
        TransformHelper::Vector2 p(static_cast<float>(pointPos.x), static_cast<float>(pointPos.y));
        TransformHelper::Vector2 originPos = TransformHelper::GetOriginScreenPoint(p, transformMat);
        return PointInfo { static_cast<uint32_t>(originPos.x_), static_cast<uint32_t>(originPos.y_) };
    }

    // This method is used to update transform when rect changed, but world transform matrix should not change.
    static void GetTransformFromWorldMat4(const TransformHelper::Matrix4& inWorldMat, const Rect& rect,
        Transform& transform)
    {
        TransformHelper::Vector3 pivotPos = { rect.posX_ + transform.pivotX_ * rect.width_,
            rect.posY_ + transform.pivotY_ * rect.height_, 0 };
        TransformHelper::Matrix4 worldMat = TransformHelper::CreateTranslation(pivotPos) * inWorldMat *
                        TransformHelper::CreateTranslation(-pivotPos);
        auto scale = worldMat.GetScale();
        auto translation = worldMat.GetTranslation();
        transform.scaleX_ = scale.x_;
        transform.scaleY_ = scale.y_;
        transform.scaleZ_ = scale.z_;
        transform.translateX_ = translation.x_;
        transform.translateY_ = translation.y_;
        transform.translateZ_ = translation.z_;
    }

    static TransformHelper::Matrix4 ComputeWorldTransformMat4(const Transform& transform)
    {
        TransformHelper::Matrix4 ret = TransformHelper::Matrix4::Identity;
        // set scale
        if (!MathHelper::NearZero(transform.scaleX_ - 1.0f) || !MathHelper::NearZero(transform.scaleY_ - 1.0f) ||
            !MathHelper::NearZero(transform.scaleZ_ - 1.0f)) {
            ret *= TransformHelper::CreateScale(transform.scaleX_, transform.scaleY_, transform.scaleZ_);
        }
        // set rotation
        if (!MathHelper::NearZero(transform.rotationX_)) {
            ret *= TransformHelper::CreateRotationX(MathHelper::ToRadians(transform.rotationX_));
        }
        if (!MathHelper::NearZero(transform.rotationY_)) {
            ret *= TransformHelper::CreateRotationY(MathHelper::ToRadians(transform.rotationY_));
        }
        if (!MathHelper::NearZero(transform.rotationZ_)) {
            ret *= TransformHelper::CreateRotationZ(MathHelper::ToRadians(transform.rotationZ_));
        }
        // set translation
        if (!MathHelper::NearZero(transform.translateX_) || !MathHelper::NearZero(transform.translateY_) ||
            !MathHelper::NearZero(transform.translateZ_)) {
            ret *= TransformHelper::CreateTranslation(TransformHelper::Vector3(transform.translateX_,
                transform.translateY_, transform.translateZ_));
        }
        return ret;
    }

    // Transform rect by matrix and get the circumscribed rect
    static Rect TransformRect(const TransformHelper::Matrix4& transformMat, const Rect& rect)
    {
        TransformHelper::Vector3 a = TransformHelper::TransformWithPerspDiv(
            TransformHelper::Vector3(rect.posX_, rect.posY_, 0), transformMat);
        TransformHelper::Vector3 b = TransformHelper::TransformWithPerspDiv(
            TransformHelper::Vector3(rect.posX_ + rect.width_, rect.posY_, 0), transformMat);
        TransformHelper::Vector3 c = TransformHelper::TransformWithPerspDiv(
            TransformHelper::Vector3(rect.posX_, rect.posY_ + rect.height_, 0), transformMat);
        TransformHelper::Vector3 d = TransformHelper::TransformWithPerspDiv(
            TransformHelper::Vector3(rect.posX_ + rect.width_, rect.posY_ + rect.height_, 0), transformMat);
        // Return smallest rect involve transformed rect(abcd)
        int32_t xmin = MathHelper::Min(a.x_, b.x_, c.x_, d.x_);
        int32_t ymin = MathHelper::Min(a.y_, b.y_, c.y_, d.y_);
        int32_t xmax = MathHelper::Max(a.x_, b.x_, c.x_, d.x_);
        int32_t ymax = MathHelper::Max(a.y_, b.y_, c.y_, d.y_);
        uint32_t w = static_cast<uint32_t>(xmax - xmin);
        uint32_t h = static_cast<uint32_t>(ymax - ymin);
        return Rect { xmin, ymin, w, h };
    }

    static TransformHelper::Vector2 CalculateHotZoneScale(const TransformHelper::Matrix4& transformMat)
    {
        TransformHelper::Vector2 hotZoneScale;
        TransformHelper::Vector3 a = TransformHelper::TransformWithPerspDiv(TransformHelper::Vector3(0, 0, 0),
            transformMat);
        TransformHelper::Vector3 b = TransformHelper::TransformWithPerspDiv(TransformHelper::Vector3(1, 0, 0),
            transformMat);
        TransformHelper::Vector3 c = TransformHelper::TransformWithPerspDiv(TransformHelper::Vector3(0, 1, 0),
            transformMat);
        TransformHelper::Vector2 axy(a.x_, a.y_);
        TransformHelper::Vector2 bxy(b.x_, b.y_);
        TransformHelper::Vector2 cxy(c.x_, c.y_);
        hotZoneScale.x_ = (axy - bxy).Length();
        hotZoneScale.y_ = (axy - cxy).Length();
        if (std::isnan(hotZoneScale.x_) || std::isnan(hotZoneScale.y_) ||
            MathHelper::NearZero(hotZoneScale.x_) || MathHelper::NearZero(hotZoneScale.y_)) {
            return TransformHelper::Vector2(1, 1);
        } else {
            return hotZoneScale;
        }
    }

    static bool CalculateTouchHotAreas(const Rect& windowRect, const std::vector<Rect>& requestRects,
        std::vector<Rect>& outRects)
    {
        bool isOk = true;
        for (const auto& rect : requestRects) {
            if (rect.posX_ < 0 || rect.posY_ < 0 || rect.width_ == 0 || rect.height_ == 0) {
                return false;
            }
            Rect hotArea;
            if (rect.posX_ >= static_cast<int32_t>(windowRect.width_) ||
                rect.posY_ >= static_cast<int32_t>(windowRect.height_)) {
                isOk = false;
                continue;
            }
            hotArea.posX_ = windowRect.posX_ + rect.posX_;
            hotArea.posY_ = windowRect.posY_ + rect.posY_;
            hotArea.width_ = static_cast<uint32_t>(std::min(hotArea.posX_ + rect.width_,
                windowRect.posX_ + windowRect.width_) - hotArea.posX_);
            hotArea.height_ = static_cast<uint32_t>(std::min(hotArea.posY_ + rect.height_,
                windowRect.posY_ + windowRect.height_) - hotArea.posY_);
            outRects.emplace_back(hotArea);
        }
        return isOk;
    }

    static bool IsRectSatisfiedWithSizeLimits(const Rect& rect, const WindowLimits& sizeLimits)
    {
        if (rect.height_ == 0) {
            return false;
        }
        auto curRatio = static_cast<float>(rect.width_) / static_cast<float>(rect.height_);
        if (sizeLimits.minWidth_ <= rect.width_ && rect.width_ <= sizeLimits.maxWidth_ &&
            sizeLimits.minHeight_ <= rect.height_ && rect.height_ <= sizeLimits.maxHeight_ &&
            sizeLimits.minRatio_ <= curRatio && curRatio <= sizeLimits.maxRatio_) {
            return true;
        }
        return false;
    }

    static bool IsOnlySupportSplitAndShowWhenLocked(bool isShowWhenLocked, uint32_t windowModeSupportType)
    {
        uint32_t splitMode = (WindowModeSupport::WINDOW_MODE_SUPPORT_SPLIT_PRIMARY |
                              WindowModeSupport::WINDOW_MODE_SUPPORT_SPLIT_SECONDARY);
        if (isShowWhenLocked && (splitMode == windowModeSupportType)) {
            return true;
        }
        return false;
    }

    static bool IsInvalidWindowInTileLayoutMode(uint32_t supportModeInfo, WindowLayoutMode layoutMode)
    {
        if ((!IsWindowModeSupported(supportModeInfo, WindowMode::WINDOW_MODE_FLOATING)) &&
            (layoutMode == WindowLayoutMode::TILE)) {
            return true;
        }
        return false;
    }

    static bool CheckSupportWindowMode(WindowMode winMode, uint32_t windowModeSupportType,
        const sptr<WindowTransitionInfo>& info)
    {
        if (!WindowHelper::IsMainWindow(info->GetWindowType())) {
            return true;
        }

        if (!IsWindowModeSupported(windowModeSupportType, winMode) ||
            IsOnlySupportSplitAndShowWhenLocked(info->GetShowFlagWhenLocked(), windowModeSupportType)) {
            return false;
        }
        return true;
    }

    static bool IsAspectRatioSatisfiedWithSizeLimits(const WindowLimits& sizeLimits, float ratio, float vpr)
    {
        /*
         * 1) Usually the size limits won't be empty after show window.
         *    In case of SetAspectRatio is called befor show (size limits may be empty at that time) or the
         *    sizeLimits is empty, there is no need to check ratio (layout will check), return true directly.
         * 2) ratio : 0.0 means reset aspect ratio
         */
        if (sizeLimits.IsEmpty() || MathHelper::NearZero(ratio)) {
            return true;
        }

        uint32_t winFrameW = static_cast<uint32_t>(WINDOW_FRAME_WIDTH * vpr) * 2; // 2 mean double decor width
        uint32_t winFrameH = static_cast<uint32_t>(WINDOW_FRAME_WIDTH * vpr) +
            static_cast<uint32_t>(WINDOW_TITLE_BAR_HEIGHT * vpr); // decor height
        uint32_t maxWidth = sizeLimits.maxWidth_ - winFrameW;
        uint32_t minWidth = sizeLimits.minWidth_ - winFrameW;
        uint32_t maxHeight = sizeLimits.maxHeight_ - winFrameH;
        uint32_t minHeight = sizeLimits.minHeight_ - winFrameH;
        float maxRatio = static_cast<float>(maxWidth) / static_cast<float>(minHeight);
        float minRatio = static_cast<float>(minWidth) / static_cast<float>(maxHeight);
        if (maxRatio < ratio || ratio < minRatio) {
            return false;
        }
        return true;
    }

    static bool IsWindowFollowParent(WindowType type)
    {
        if (type == WindowType::WINDOW_TYPE_DIALOG || type == WindowType::WINDOW_TYPE_APP_SUB_WINDOW) {
            return true;
        }
        return false;
    }

    static bool CheckButtonStyleValid(const DecorButtonStyle& decorButtonStyle)
    {
        return decorButtonStyle.buttonBackgroundSize >= MIN_BUTTON_BACKGROUND_SIZE &&
               decorButtonStyle.buttonBackgroundSize <= MAX_BUTTON_BACKGROUND_SIZE &&
               decorButtonStyle.closeButtonRightMargin >= MIN_CLOSE_BUTTON_RIGHT_MARGIN &&
               decorButtonStyle.closeButtonRightMargin <= MAX_CLOSE_BUTTON_RIGHT_MARGIN &&
               decorButtonStyle.spacingBetweenButtons >= MIN_SPACING_BETWEEN_BUTTONS &&
               decorButtonStyle.spacingBetweenButtons <= MAX_SPACING_BETWEEN_BUTTONS &&
               decorButtonStyle.colorMode >= MIN_COLOR_MODE &&
               decorButtonStyle.colorMode <= MAX_COLOR_MODE &&
               decorButtonStyle.buttonIconSize >= MIN_BUTTON_ICON_SIZE &&
               decorButtonStyle.buttonIconSize <= MAX_BUTTON_ICON_SIZE &&
               decorButtonStyle.buttonBackgroundCornerRadius >= MIN_BUTTON_BACKGROUND_CORNER_RADIUS &&
               decorButtonStyle.buttonBackgroundCornerRadius <= MAX_BUTTON_BACKGROUND_CORNER_RADIUS;
    }

    static void SplitStringByDelimiter(
        const std::string& inputStr, const std::string& delimiter, std::unordered_set<std::string>& container)
    {
        if (inputStr.empty()) {
            return;
        }
        if (delimiter.empty()) {
            container.insert(inputStr);
            return;
        }
        std::string::size_type start = 0;
        std::string::size_type end = 0;
        while ((end = inputStr.find(delimiter, start)) != std::string::npos) {
            container.insert(inputStr.substr(start, end - start));
            start = end + delimiter.length();
        }
        container.insert(inputStr.substr(start));
    }

    /**
     * @brief Check whether the window has scaling applied.
     *
     * This method returns true if scaleX or scaleY is not equal to 1,
     * meaning the window is currently scaled.
     *
     * @param transform The layout transform of the window.
     * @return true if the window is scaled, false otherwise.
     */
    static bool IsScaled(const Transform& transform)
    {
        auto IsApproximatelyOne = [](float value) {
            constexpr float EPSILON = 1e-6f;
            return std::fabs(value - 1.0f) < EPSILON;
        };
        return !IsApproximatelyOne(transform.scaleX_) || !IsApproximatelyOne(transform.scaleY_);
    }

    static bool IsSupportSetZIndexWindow(WindowType windowType)
    {
        return windowType == WindowType::WINDOW_TYPE_WALLET_SWIPE_CARD ||
               windowType == WindowType::WINDOW_TYPE_VOICE_INTERACTION ||
               windowType == WindowType::WINDOW_TYPE_SCREENSHOT ||
               windowType == WindowType::WINDOW_TYPE_SCREEN_CONTROL ||
               windowType == WindowType::WINDOW_TYPE_FLOAT_NAVIGATION ||
               windowType == WindowType::WINDOW_TYPE_MUTISCREEN_COLLABORATION;
    }

private:
    WindowHelper() = default;
    ~WindowHelper() = default;
};
} // namespace OHOS
} // namespace Rosen
#endif // OHOS_WM_INCLUDE_WM_HELPER_H
