/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "window_listener.h"
#include <cstdint>
#include "cj_lambda.h"
#include "event_handler.h"
#include "event_runner.h"
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
CjWindowListener::CjWindowListener(int64_t callbackObject, CaseType caseType)
    : caseType_(caseType)
{
    TLOGI(WmsLogTag::DEFAULT, "Create CjWindowListener");
    weakRef_ = wptr<CjWindowListener>(this);
    auto func = reinterpret_cast<void(*)(void*)>(callbackObject);
    cjCallBack_ = CJLambda::Create(func);
}

CjWindowListener::~CjWindowListener()
{
    TLOGI(WmsLogTag::WMS_DIALOG, "~CjWindowListener");
}

void CjWindowListener::SetMainEventHandler()
{
    auto mainRunner = AppExecFwk::EventRunner::GetMainEventRunner();
    if (mainRunner == nullptr) {
        return;
    }
    eventHandler_ = std::make_shared<AppExecFwk::EventHandler>(mainRunner);
}

void CjWindowListener::CallCjMethod(const char* methodName, void* argv)
{
    TLOGD(WmsLogTag::WMS_DIALOG, "[WindowListener]methodName=%{public}s", methodName);
    if (cjCallBack_ == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "[WindowListener]env_ nullptr or jsCallBack_ is nullptr");
        return;
    }
    cjCallBack_(argv);
}

CjRectInfo GetCjRect(const Rect& rect)
{
    CjRectInfo result = {
        .left = rect.posX_,
        .top = rect.posY_,
        .width = rect.width_,
        .height = rect.height_
    };
    return result;
}

void CjWindowListener::OnSystemBarPropertyChange(DisplayId displayId, const SystemBarRegionTints& tints)
{
    auto thisListener = weakRef_.promote();
    if (thisListener == nullptr) {
        return;
    }

    std::vector<SystemBarRegionTintInfo> tintInfos;
    for (const auto& tint : tints) {
        auto actType = CJ_TO_WINDOW_TYPE_MAP.count(tint.type_) != 0 ?
            CJ_TO_WINDOW_TYPE_MAP.at(tint.type_) : static_cast<ApiWindowType>(tint.type_);
        SystemBarRegionTintInfo info = {
            .type = static_cast<uint32_t>(actType),
            .isEnable = tint.prop_.enable_,
            .backgroundColor = tint.prop_.backgroundColor_,
            .contentColor = tint.prop_.contentColor_,
            .region = GetCjRect(tint.region_)
        };
        tintInfos.push_back(info);
    }

    struct SystemBarPropertyInfo {
        uint32_t displayId;
        std::vector<SystemBarRegionTintInfo> tints;
    };

    SystemBarPropertyInfo propertyInfo = {
        .displayId = static_cast<uint32_t>(displayId),
        .tints = tintInfos
    };

    thisListener->CallCjMethod(SYSTEM_BAR_TINT_CHANGE_CB.c_str(), &propertyInfo);
}

void CjWindowListener::OnSizeChange(Rect rect, WindowSizeChangeReason reason,
    const std::shared_ptr<RSTransaction>& rsTransaction)
{
    if (currRect_.width_ == rect.width_ && currRect_.height_ == rect.height_ &&
        reason != WindowSizeChangeReason::DRAG_END) {
        return;
    }
    auto thisListener = weakRef_.promote();
    if (thisListener == nullptr) {
        return;
    }
    struct SizeChangeInfo {
        uint32_t width;
        uint32_t height;
    };
    SizeChangeInfo argv = {
        .width = rect.width_,
        .height = rect.height_
    };
    thisListener->CallCjMethod(WINDOW_SIZE_CHANGE_CB.c_str(), &argv);
    currRect_ = rect;
}

void CjWindowListener::OnModeChange(WindowMode mode, bool hasDeco)
{
}

void CjWindowListener::OnAvoidAreaChanged(const AvoidArea avoidArea, AvoidAreaType type,
    const sptr<OccupiedAreaChangeInfo>& info)
{
    auto thisListener = weakRef_.promote();
    if (thisListener == nullptr) {
        return;
    }

    struct AvoidAreaValue {
        bool visible;
        CjRectInfo leftRect;
        CjRectInfo topRect;
        CjRectInfo rightRect;
        CjRectInfo bottomRect;
    };
    struct AvoidAreaInfo {
        uint32_t type;
        AvoidAreaValue area;
    };
    AvoidAreaInfo avoidAreaInfo = {
        .type = static_cast<uint32_t>(type),
        .area = {
            .visible = (type != AvoidAreaType::TYPE_CUTOUT),
            .leftRect = GetCjRect(avoidArea.leftRect_),
            .topRect = GetCjRect(avoidArea.topRect_),
            .rightRect = GetCjRect(avoidArea.rightRect_),
            .bottomRect = GetCjRect(avoidArea.bottomRect_)
        }
    };

    thisListener->CallCjMethod(AVOID_AREA_CHANGE_CB.c_str(), &avoidAreaInfo);
}

void CjWindowListener::AfterForeground()
{
    if (state_ == WindowState::STATE_INITIAL || state_ == WindowState::STATE_HIDDEN) {
        LifeCycleCallBack(LifeCycleEventType::FOREGROUND);
        state_ = WindowState::STATE_SHOWN;
    }
}

void CjWindowListener::AfterBackground()
{
    if (state_ == WindowState::STATE_INITIAL || state_ == WindowState::STATE_SHOWN) {
        LifeCycleCallBack(LifeCycleEventType::BACKGROUND);
        state_ = WindowState::STATE_HIDDEN;
    }
}

void CjWindowListener::AfterFocused()
{
    LifeCycleCallBack(LifeCycleEventType::ACTIVE);
}

void CjWindowListener::AfterUnfocused()
{
    LifeCycleCallBack(LifeCycleEventType::INACTIVE);
}

void CjWindowListener::AfterResumed()
{
    if (caseType_ == CaseType::CASE_STAGE) {
        LifeCycleCallBack(LifeCycleEventType::RESUMED);
    }
}

void CjWindowListener::AfterPaused()
{
    if (caseType_ == CaseType::CASE_STAGE) {
        LifeCycleCallBack(LifeCycleEventType::PAUSED);
    }
}

void CjWindowListener::AfterDestroyed()
{
    if (caseType_ == CaseType::CASE_WINDOW) {
        LifeCycleCallBack(LifeCycleEventType::DESTROYED);
    }
}

void CjWindowListener::LifeCycleCallBack(LifeCycleEventType eventType)
{
    uint32_t type = static_cast<uint32_t>(eventType);
    auto thisListener = weakRef_.promote();
    if (thisListener == nullptr) {
        return;
    }

    thisListener->CallCjMethod(LIFECYCLE_EVENT_CB.c_str(), &type);
}

void CjWindowListener::OnSizeChange(const sptr<OccupiedAreaChangeInfo>& info,
    const std::shared_ptr<RSTransaction>& rsTransaction)
{
    if (info == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "[WindowListener] this changeInfo is nullptr");
        return;
    }
    TLOGI(WmsLogTag::WMS_DIALOG, "[WindowListener]OccupiedAreaChangeInfo, type: %{public}u, " \
        "input rect: [%{public}d, %{public}d, %{public}u, %{public}u]",
        static_cast<uint32_t>(info->type_),
        info->rect_.posX_, info->rect_.posY_, info->rect_.width_, info->rect_.height_);
    auto thisListener = weakRef_.promote();
    if (thisListener == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "[WindowListener] this listener is nullptr");
        return;
    }
    void* argv = &(info->rect_.height_);
    thisListener->CallCjMethod(KEYBOARD_HEIGHT_CHANGE_CB.c_str(), argv);
}

void CjWindowListener::OnTouchOutside() const
{
    auto thisListener = weakRef_.promote();
    if (thisListener == nullptr) {
        return;
    }

    thisListener->CallCjMethod(TOUCH_OUTSIDE_CB.c_str(), nullptr);
}

void CjWindowListener::OnScreenshot()
{
    auto thisListener = weakRef_.promote();
    if (thisListener == nullptr) {
        return;
    }

    thisListener->CallCjMethod(SCREENSHOT_EVENT_CB.c_str(), nullptr);
}

void CjWindowListener::OnDialogTargetTouch() const
{
    auto thisListener = weakRef_.promote();
    if (thisListener == nullptr) {
        return;
    }

    thisListener->CallCjMethod(DIALOG_TARGET_TOUCH_CB.c_str(), nullptr);
}

void CjWindowListener::OnDialogDeathRecipient() const
{
    auto thisListener = weakRef_.promote();
    if (thisListener == nullptr) {
        return;
    }

    thisListener->CallCjMethod(DIALOG_DEATH_RECIPIENT_CB.c_str(), nullptr);
}

void CjWindowListener::OnGestureNavigationEnabledUpdate(bool enable)
{
    auto thisListener = weakRef_.promote();
    if (thisListener == nullptr) {
        return;
    }

    bool navigationEnabled = enable;
    thisListener->CallCjMethod(GESTURE_NAVIGATION_ENABLED_CHANGE_CB.c_str(), &navigationEnabled);
}

void CjWindowListener::OnWaterMarkFlagUpdate(bool showWaterMark)
{
    auto thisListener = weakRef_.promote();
    if (thisListener == nullptr) {
        return;
    }

    bool waterMarkFlag = showWaterMark;
    thisListener->CallCjMethod(WATER_MARK_FLAG_CHANGE_CB.c_str(), &waterMarkFlag);
}

void CjWindowListener::OnApplicationFocusUpdate(bool isFocused)
{
    auto thisListener = weakRef_.promote();
    if (thisListener == nullptr) {
        return;
    }

    bool isFocusFlag = isFocused;
    thisListener->CallCjMethod(APPLICATION_FOCUS_STATE_CHANGE_CB.c_str(), &isFocusFlag);
}

void CjWindowListener::OnWindowVisibilityChangedCallback(const bool isVisible)
{
    auto thisListener = weakRef_.promote();
    if (thisListener == nullptr) {
        return;
    }

    bool visibility = isVisible;
    thisListener->CallCjMethod(WINDOW_VISIBILITY_CHANGE_CB.c_str(), &visibility);
}

void CjWindowListener::OnWindowStatusChange(WindowStatus status)
{
    auto thisListener = weakRef_.promote();
    if (thisListener == nullptr) {
        return;
    }

    WindowStatus currentStatus = status;
    thisListener->CallCjMethod(WINDOW_STATUS_CHANGE_CB.c_str(), &currentStatus);
}

void CjWindowListener::SetTimeout(int64_t timeout)
{
    noInteractionTimeout_ = timeout;
}

int64_t CjWindowListener::GetTimeout() const
{
    return noInteractionTimeout_;
}

void CjWindowListener::OnWindowNoInteractionCallback()
{
    auto thisListener = weakRef_.promote();
    if (thisListener == nullptr) {
        return;
    }
    thisListener->CallCjMethod(WINDOW_NO_INTERACTION_DETECTED_CB.c_str(), nullptr);
}

void CjWindowListener::OnWindowTitleButtonRectChanged(const TitleButtonRect& titleButtonRect)
{
    auto thisListener = weakRef_.promote();
    if (thisListener == nullptr) {
        return;
    }
    struct TitleButtonRectInfo {
        int32_t right;
        int32_t top;
        uint32_t width;
        uint32_t height;
    };
    TitleButtonRectInfo argv = {
        .right = titleButtonRect.posX_,
        .top = titleButtonRect.posY_,
        .width = titleButtonRect.width_,
        .height = titleButtonRect.height_
    };
    thisListener->CallCjMethod(WINDOW_TITLE_BUTTON_RECT_CHANGE_CB.c_str(), &argv);
}

void CjWindowListener::OnRectChange(Rect rect, WindowSizeChangeReason reason)
{
    if (currRect_ == rect && reason == WindowSizeChangeReason::UNDEFINED) {
        return;
    }
    RectChangeReason rectChangReason = RectChangeReason::UNDEFINED;
    if (CJ_SIZE_CHANGE_REASON.count(reason) != 0 &&
        !(reason == WindowSizeChangeReason::MAXIMIZE && rect.posX_ != 0)) {
        rectChangReason = CJ_SIZE_CHANGE_REASON.at(reason);
    }
    if (currentReason_ != RectChangeReason::DRAG && rectChangReason == RectChangeReason::DRAG_END) {
        rectChangReason = RectChangeReason::MOVE;
    }

    auto thisListener = weakRef_.promote();
    if (thisListener == nullptr) {
        return;
    }

    struct RectChangeInfo {
        CjRectInfo rect;
        WindowSizeChangeReason reason;
    };

    RectChangeInfo rectChangeInfo = {
        .rect = GetCjRect(rect),
        .reason = reason
    };

    thisListener->CallCjMethod(WINDOW_RECT_CHANGE_CB.c_str(), &rectChangeInfo);
    currRect_ = rect;
}

void CjWindowListener::OnSubWindowClose(bool& terminateCloseProcess)
{
    auto thisListener = weakRef_.promote();
    if (thisListener == nullptr) {
        return;
    }

    bool value = terminateCloseProcess;
    thisListener->CallCjMethod(WINDOW_SUB_WINDOW_CLOSE_CB.c_str(), &value);
    terminateCloseProcess = value;
}

void CjWindowListener::OnMainWindowClose(bool& terminateCloseProcess)
{
    auto thisListener = weakRef_.promote();
    if (thisListener == nullptr) {
        return;
    }

    bool value = terminateCloseProcess;
    thisListener->CallCjMethod(WINDOW_STAGE_CLOSE_CB.c_str(), &value);
    terminateCloseProcess = value;
}

}
}
