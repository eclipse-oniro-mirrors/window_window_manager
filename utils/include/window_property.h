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

#ifndef OHOS_ROSEN_WINDOW_PROPERTY_H
#define OHOS_ROSEN_WINDOW_PROPERTY_H

#include <refbase.h>
#include <string>
#include <unordered_map>
#include <parcel.h>
#include "class_var_definition.h"
#include "pointer_event.h"
#include "dm_common.h"
#include "wm_common.h"
#include "wm_common_inner.h"
#include "wm_math.h"

namespace OHOS {
namespace Rosen {
class WindowProperty : public Parcelable {
public:
    WindowProperty() = default;
    explicit WindowProperty(const sptr<WindowProperty>& property);
    ~WindowProperty() = default;

    void CopyFrom(const sptr<WindowProperty>& property);

    void SetWindowName(const std::string& name);
    void SetAbilityInfo(const AbilityInfo& info);
    void SetRequestRect(const struct Rect& rect);
    void SetWindowRect(const struct Rect& rect);
    void SetDecoStatus(bool decoStatus);
    void SetWindowHotZoneRect(const struct Rect& rect);
    void SetWindowType(WindowType type);
    void SetWindowMode(WindowMode mode);
    void SetLastWindowMode(WindowMode mode);
    void ResumeLastWindowMode();
    void SetFullScreen(bool isFullScreen);
    void SetFocusable(bool isFocusable);
    void SetTouchable(bool isTouchable);
    void SetPrivacyMode(bool isPrivate);
    void SetSystemPrivacyMode(bool isSystemPrivate);
    void SetSnapshotSkip(bool isSkip);
    void SetTransparent(bool isTransparent);
    void SetAlpha(float alpha);
    void SetBrightness(float brightness);
    void SetTurnScreenOn(bool turnScreenOn);
    void SetKeepScreenOn(bool keepScreenOn);
    void ChangeCallingWindowId(uint32_t windowId);
    void SetDisplayId(DisplayId displayId);
    void SetWindowId(uint32_t windowId);
    void SetParentId(uint32_t parentId);
    void SetWindowFlags(uint32_t flags);
    void AddWindowFlag(WindowFlag flag);
    void SetSystemBarProperty(WindowType type, const SystemBarProperty& state);
    void SetDecorEnable(bool decorEnable);
    void SetHitOffset(const PointInfo& offset);
    void SetAnimationFlag(uint32_t animationFlag);
    void SetWindowSizeChangeReason(WindowSizeChangeReason reason);
    void SetTokenState(bool hasToken);
    void SetWindowModeSupportType(uint32_t windowModeSupportType);
    void SetRequestWindowModeSupportType(uint32_t requestWindowModeSupportType);
    void SetDragType(DragType dragType);
    void SetStretchable(bool stretchable);
    void SetOriginRect(const Rect& rect);
    void SetTouchHotAreas(const std::vector<Rect>& rects);
    void SetAccessTokenId(uint32_t accessTokenId);
    void SetSizeLimits(const WindowLimits& sizeLimits);
    void SetUpdatedSizeLimits(const WindowLimits& sizeLimits);
    WindowSizeChangeReason GetWindowSizeChangeReason() const;
    void SetTransform(const Transform& trans);
    void ComputeTransform();
    void SetZoomTransform(const Transform& trans);
    void SetDisplayZoomState(bool isDisplayZoomOn);
    void SetAnimateWindowFlag(bool isAnimateWindow);
    void UpdatePointerEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent);
    bool isNeedComputerTransform();
    void ClearTransformZAxisOffset(Transform& trans);
    void SetAspectRatio(float ratio);
    void SetWindowGravity(WindowGravity gravity, uint32_t percent);
    void SetMaximizeMode(MaximizeMode maximizeMode);

    const std::string& GetWindowName() const;
    const AbilityInfo& GetAbilityInfo() const;
    Rect GetRequestRect() const;
    Rect GetWindowRect() const;
    bool GetDecoStatus() const;
    Rect GetWindowHotZoneRect() const;
    WindowType GetWindowType() const;
    WindowMode GetWindowMode() const;
    WindowMode GetLastWindowMode() const;
    bool GetFullScreen() const;
    bool GetFocusable() const;
    bool GetTouchable() const;
    uint32_t GetCallingWindow() const;
    bool GetPrivacyMode() const;
    bool GetSystemPrivacyMode() const;
    bool GetSnapshotSkip() const;
    bool GetTransparent() const;
    bool GetTokenState() const;
    float GetAlpha() const;
    float GetBrightness() const;
    bool IsTurnScreenOn() const;
    bool IsKeepScreenOn() const;
    DisplayId GetDisplayId() const;
    uint32_t GetWindowId() const;
    uint32_t GetParentId() const;
    uint32_t GetWindowFlags() const;
    const std::unordered_map<WindowType, SystemBarProperty>& GetSystemBarProperty() const;
    bool GetDecorEnable() const;
    const PointInfo& GetHitOffset() const;
    uint32_t GetAnimationFlag() const;
    uint32_t GetWindowModeSupportType() const;
    uint32_t GetRequestWindowModeSupportType() const;
    DragType GetDragType() const;
    bool GetStretchable() const;
    const Rect& GetOriginRect() const;
    void GetTouchHotAreas(std::vector<Rect>& rects) const;
    uint32_t GetAccessTokenId() const;
    const Transform& GetTransform() const;
    const Transform& GetZoomTransform() const;
    bool IsDisplayZoomOn() const;
    bool IsAnimateWindow() const;
    WindowLimits GetSizeLimits() const;
    WindowLimits GetUpdatedSizeLimits() const;
    const TransformHelper::Matrix4& GetTransformMat() const;
    const TransformHelper::Matrix4& GetWorldTransformMat() const;
    float GetAspectRatio() const;
    void GetWindowGravity(WindowGravity& gravity, uint32_t& percent) const;
    MaximizeMode GetMaximizeMode() const;

    virtual bool Marshalling(Parcel& parcel) const override;
    static WindowProperty* Unmarshalling(Parcel& parcel);

    bool Write(Parcel& parcel, PropertyChangeAction action);
    void Read(Parcel& parcel, PropertyChangeAction action);
    bool isSystemCalling_ { false };

    void SetTextFieldPositionY(double textFieldPositionY);
    void SetTextFieldHeight(double textFieldHeight);

    double GetTextFieldPositionY() const;
    double GetTextFieldHeight() const;

    void SetFollowScreenChange(bool isFollowScreenChange);
    bool GetFollowScreenChange() const;

private:
    bool MapMarshalling(Parcel& parcel) const;
    static void MapUnmarshalling(Parcel& parcel, WindowProperty* property);
    bool MarshallingTouchHotAreas(Parcel& parcel) const;
    static void UnmarshallingTouchHotAreas(Parcel& parcel, WindowProperty* property);
    bool MarshallingTransform(Parcel& parcel) const;
    static void UnmarshallingTransform(Parcel& parcel, WindowProperty* property);
    bool MarshallingWindowSizeLimits(Parcel& parcel) const;
    static void UnmarshallingWindowSizeLimits(Parcel& parcel, WindowProperty* property);
    void HandleComputeTransform(const Transform& trans);

    std::string windowName_;
    AbilityInfo abilityInfo_;
    Rect requestRect_ { 0, 0, 0, 0 }; // window rect requested by the client (without decoration size)
    Rect windowRect_ { 0, 0, 0, 0 }; // actual window rect
    bool decoStatus_ { false }; // window has been decorated or not
    WindowType type_ { WindowType::WINDOW_TYPE_APP_MAIN_WINDOW };
    WindowMode mode_ { WindowMode::WINDOW_MODE_UNDEFINED };
    WindowMode lastMode_ { WindowMode::WINDOW_MODE_UNDEFINED };
    uint32_t flags_ { 0 };
    bool isFullScreen_ { true };
    bool focusable_ { true };
    bool touchable_ { true };
    bool isPrivacyMode_ { false };
    bool isSystemPrivacyMode_ { false };
    bool isTransparent_ { false };
    bool tokenState_ { false };
    float alpha_ { 1.0f };
    float brightness_ = UNDEFINED_BRIGHTNESS;
    bool turnScreenOn_ = false;
    bool keepScreenOn_ = false;
    bool isFollowScreenChange_ = false;
    uint32_t callingWindow_ = INVALID_WINDOW_ID;
    DisplayId displayId_ { 0 };
    uint32_t windowId_ = INVALID_WINDOW_ID;
    uint32_t parentId_ = INVALID_WINDOW_ID;
    PointInfo hitOffset_ { 0, 0 };
    uint32_t animationFlag_ { static_cast<uint32_t>(WindowAnimation::DEFAULT) };
    // windowModeSupportType_ means supported modes in runtime, which can be changed
    uint32_t windowModeSupportType_ {WindowModeSupport::WINDOW_MODE_SUPPORT_ALL};
    // requestWindowModeSupportType_ is configured in abilityInfo, usually can't be changed
    uint32_t requestWindowModeSupportType_ {WindowModeSupport::WINDOW_MODE_SUPPORT_ALL};
    WindowSizeChangeReason windowSizeChangeReason_ = WindowSizeChangeReason::UNDEFINED;
    std::unordered_map<WindowType, SystemBarProperty> sysBarPropMap_ {
        { WindowType::WINDOW_TYPE_STATUS_BAR,     SystemBarProperty() },
        { WindowType::WINDOW_TYPE_NAVIGATION_BAR, SystemBarProperty() },
    };
    bool isDecorEnable_ { true };
    Rect originRect_ = { 0, 0, 0, 0 };
    bool isStretchable_ {false};
    DragType dragType_ = DragType::DRAG_UNDEFINED;
    std::vector<Rect> touchHotAreas_;  // coordinates relative to window.
    uint32_t accessTokenId_ { 0 };
    // Transform info
    Transform trans_;
    bool recomputeTransformMat_ { false };
    TransformHelper::Matrix4 transformMat_ = TransformHelper::Matrix4::Identity;
    TransformHelper::Matrix4 worldTransformMat_ = TransformHelper::Matrix4::Identity;
    // Display Zoom transform info
    Transform zoomTrans_; // Compared with original window rect, including class member trans_
    bool reCalcuZoomTransformMat_ {true};
    // if scale of trans_ is less than 1.0, zoomTrans_ may be an identity matrix
    bool isDisplayZoomOn_ {false};
    bool isAnimateWindow_ {false};
    float aspectRatio_ {0.0};
    WindowGravity windowGravity_ = WindowGravity::WINDOW_GRAVITY_BOTTOM;
    uint32_t windowGravitySizePercent_ = 0;
    bool isSnapshotSkip_ = false;

    DEFINE_VAR_DEFAULT_FUNC_GET_SET(Orientation, RequestedOrientation, requestedOrientation, Orientation::UNSPECIFIED);
    DEFINE_VAR_DEFAULT_FUNC_GET_SET(uint32_t, ApiCompatibleVersion, apiCompatibleVersion, 0u);
    WindowLimits sizeLimits_;
    WindowLimits updatedSizeLimits_;
    MaximizeMode maximizeMode_ { MaximizeMode::MODE_RECOVER };

    double textFieldPositionY_ = 0.0;
    double textFieldHeight_ = 0.0;
};
}
}
#endif // OHOS_ROSEN_WINDOW_PROPERTY_H
