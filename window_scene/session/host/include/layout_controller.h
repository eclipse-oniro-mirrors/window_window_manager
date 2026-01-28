/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_ROSEN_LAYOUT_CONTROLLER_H
#define OHOS_ROSEN_LAYOUT_CONTROLLER_H

#include <mutex>
#include <refbase.h>
#include <struct_multimodal.h>

#include "common/include/window_session_property.h"
#include "interfaces/include/ws_common.h"
#include "wm_common.h"

namespace OHOS::Rosen {
using GetSystemConfigFunc = std::function<SystemSessionConfig()>;

class LayoutController : public RefBase {
public:
    LayoutController(const sptr<WindowSessionProperty>& property);
    ~LayoutController() = default;

    void SetSessionRect(const WSRect& rect) { winRect_ = rect; }
    bool SetSessionGlobalRect(const WSRect& rect);
    void SetClientRect(const WSRect& rect);
    WSRect GetSessionRect() const { return winRect_; }
    WSRect GetSessionGlobalRect() const;
    WSRect GetClientRect() const;
    void GetGlobalScaledRect(Rect& globalScaledRect);
    WSRect ConvertRelativeRectToGlobal(const WSRect& relativeRect, DisplayId currentDisplayId) const;
    WSRect ConvertGlobalRectToRelative(const WSRect& globalRect, DisplayId targetDisplayId) const;
    int32_t GetSessionPersistentId() const;
    bool AdjustRectByAspectRatio(WSRect& rect, bool isDecorEnable);
    WSRect AdjustRectByAspectRatio(const WSRect& rect, const WindowDecoration& decoration);
    float GetAspectRatio() const { return aspectRatio_; }
    void SetAspectRatio(float ratio) { aspectRatio_ = ratio; }
    float GetScaleX() const { return scaleX_; }
    float GetScaleY() const { return scaleY_; }
    float GetPivotX() const { return pivotX_; }
    float GetPivotY() const { return pivotY_; }
    void UpdateSizeChangeReason(SizeChangeReason reason) { reason_ = reason; }
    SizeChangeReason GetSizeChangeReason() const { return reason_; }
    void SetScale(float scaleX, float scaleY, float pivotX, float pivotY);
    void SetClientScale(float scaleX, float scaleY, float pivotX, float pivotY);
    bool IsTransformNeedUpdate(float scaleX, float scaleY, float pivotX, float pivotY);
    void SetSystemConfigFunc(GetSystemConfigFunc&& func);

private:
    float scaleX_ = 1.0f;
    float scaleY_ = 1.0f;
    float pivotX_ = 0.0f;
    float pivotY_ = 0.0f;
    float clientScaleX_ = 1.0f;
    float clientScaleY_ = 1.0f;
    float clientPivotX_ = 0.0f;
    float clientPivotY_ = 0.0f;
    WSRect winRect_;
    WSRect clientRect_;     // rect saved when prelayout or notify client to update rect
    mutable std::mutex globalRectMutex_;
    WSRect globalRect_;     // globalRect include translate
    SizeChangeReason reason_ = SizeChangeReason::UNDEFINED;
    float aspectRatio_ = 0.0f;
    sptr<WindowSessionProperty> sessionProperty_;
    GetSystemConfigFunc getSystemConfigFunc_;

    void AdjustRectByLimits(WindowLimits limits, float ratio, bool isDecor, float vpr, WSRect& rect);
};
} // namespace OHOS::Rosen
#endif // OHOS_ROSEN_LAYOUT_CONTROLLER_H
