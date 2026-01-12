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

#ifndef OHOS_ROSEN_WINDOW_MANAGER_H
#define OHOS_ROSEN_WINDOW_MANAGER_H

#include <iremote_object.h>
#include <memory>
#include <mutex>
#include <refbase.h>
#include <shared_mutex>
#include <vector>

#include "dm_common.h"
#include "focus_change_info.h"

#include "window.h"
#include "window_drawing_content_info.h"
#include "window_pid_visibility_info.h"
#include "window_visibility_info.h"
#include "wm_common.h"
#include "wm_single_instance.h"

namespace OHOS {
namespace Rosen {
struct SystemBarRegionTint {
    WindowType type_;
    SystemBarProperty prop_;
    Rect region_;
    SystemBarRegionTint()
        : type_(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW), prop_(), region_{ 0, 0, 0, 0 } {}
    SystemBarRegionTint(WindowType type, SystemBarProperty prop, Rect region)
        : type_(type), prop_(prop), region_(region) {}
};
using SystemBarRegionTints = std::vector<SystemBarRegionTint>;
using GetJSWindowObjFunc = std::function<void*(const std::string& windowName)>;
using WindowChangeInfoType = std::variant<int32_t, uint32_t, int64_t, uint64_t, std::string, float, Rect, WindowMode,
    WindowVisibilityState, bool>;
using WindowInfoList = std::vector<std::unordered_map<WindowInfoKey, WindowChangeInfoType>>;

struct VisibleWindowNumInfo {
    uint32_t displayId;
    uint32_t visibleWindowNum;
};

struct WindowSnapshotDataPack {
    std::shared_ptr<Media::PixelMap> pixelMap = nullptr;
    WMError result = WMError::WM_OK;
};

struct WindowLifeCycleInfo {
    int32_t windowId;
    WindowType windowType;
    std::string windowName;
};

/**
 * @class IWMSConnectionChangedListener
 *
 * @brief Listener to observe window lifecycle status.
 */
class IWindowLifeCycleListener : virtual public RefBase {
public:
    /**
     * @brief Notify caller when window is destroyed
     *
     * @param lifeCycleInfo window lifecycle info.
     * @param jsWindowNapiValue js window object napi value.
     *
     */
    virtual void OnWindowDestroyed(const WindowLifeCycleInfo& lifeCycleInfo, void* jsWindowNapiValue) = 0;
};

/**
 * @class IWMSConnectionChangedListener
 *
 * @brief Listener to observe WMS connection status.
 */
class IWMSConnectionChangedListener : virtual public RefBase {
public:
    /**
     * @brief Notify caller when WMS connected
     *
     * @param userId ID of the user who has connected to the WMS.
     *
     * @param screenId ID of the screen that is connected to the WMS, screenId is currently always 0.
     */
    virtual void OnConnected(int32_t userId, int32_t screenId) = 0;

    /**
     * @brief Notify caller when WMS disconnected
     *
     * @param userId ID of the user who has disconnected to the WMS.
     *
     * @param screenId ID of the screen that is disconnected to the WMS, screenId is currently always 0.
     */
    virtual void OnDisconnected(int32_t userId, int32_t screenId) = 0;
};

/**
 * @class IFocusChangedListener
 *
 * @brief Listener to observe focus changed.
 */
class IFocusChangedListener : virtual public RefBase {
public:
    /**
     * @brief Notify caller when window get focus
     *
     * @param focusChangeInfo Window info while its focus status changed.
     */
    virtual void OnFocused(const sptr<FocusChangeInfo>& focusChangeInfo) = 0;

    /**
     * @brief Notify caller when window lose focus
     *
     * @param focusChangeInfo Window info while its focus status changed.
     */
    virtual void OnUnfocused(const sptr<FocusChangeInfo>& focusChangeInfo) = 0;
};

/**
 * @class IAllGroupInfoChangedListener
 *
 * @brief Listener to observe display added and removed.
 */
class IAllGroupInfoChangedListener : virtual public RefBase {
public:
    /**
     * @brief Notify caller when a display is connected or disconnected.
     *
     * @param displayGroupId display group id of the changed display.
     * @param displayId display id.
     * @param isAdd true means a new display is added in the current display group, false means the opposite.
     */
    virtual void OnDisplayGroupInfoChange(DisplayGroupId displayGroupId, DisplayId displayId, bool isAdd) = 0;
};

/**
 * @class IWindowModeChangedListener
 *
 * @brief Listener to observe window mode change.
 */
class IWindowModeChangedListener : virtual public RefBase {
public:
    /**
     * @brief Notify caller when window mode update.
     *
     * @param mode Window mode.
     */
    virtual void OnWindowModeUpdate(WindowModeType mode) = 0;
};

/**
 * @class ISystemBarChangedListener
 *
 * @brief Listener to observe systembar changed.
 */
class ISystemBarChangedListener : virtual public RefBase {
public:
    /**
     * @brief Notify caller when system bar property changed
     *
     * @param displayId ID of display.
     * @param tints Tint of system bar region.
     */
    virtual void OnSystemBarPropertyChange(DisplayId displayId, const SystemBarRegionTints& tints) = 0;
};

/**
 * @class IGestureNavigationEnabledChangedListener
 *
 * @brief Listener to observe GestureNavigationEnabled changed.
 */
class IGestureNavigationEnabledChangedListener : virtual public RefBase {
public:
    /**
     * @brief Notify caller when GestureNavigationEnabled changed.
     *
     * @param enable True means set Gesture on, false means set Gesture off.
     */
    virtual void OnGestureNavigationEnabledUpdate(bool enable) = 0;
};

/**
 * @class IVisibilityChangedListener
 *
 * @brief Listener to observe visibility changed.
 */
class IVisibilityChangedListener : virtual public RefBase {
public:
    /**
     * @brief Notify caller when window visibility changed.
     *
     * @param windowVisibilityInfo Window visibility info.
     */
    virtual void OnWindowVisibilityChanged(const std::vector<sptr<WindowVisibilityInfo>>& windowVisibilityInfo) = 0;
};

/**
 * @class IDrawingContentChangedListener
 *
 * @brief Listener to observe drawing content changed.
 */
class IDrawingContentChangedListener : virtual public RefBase {
public:
    /**
     * @brief Notify caller when window DrawingContent changed.
     *
     * @param windowDrawingInfo Window DrawingContent info.
     */
    virtual void OnWindowDrawingContentChanged(const std::vector<sptr<WindowDrawingContentInfo>>&
        windowDrawingInfo) = 0;
};

/**
 * @class IWindowStyleChangedListener
 *
 * @brief Listener to observe windowStyle changed.
 */
class IWindowStyleChangedListener : virtual public RefBase {
public:
    /**
     * @brief Notify caller when window style changed.
     *
     * @param styleType
     */
    virtual void OnWindowStyleUpdate(WindowStyleType styleType) = 0;
};

/**
 * @class IKeyboardCallingWindowDisplayChangedListener
 *
 * @brief Observe the display change of keyboard callingWindow.
 */
class IKeyboardCallingWindowDisplayChangedListener : virtual public RefBase {
public:
    /**
     * @brief Notify caller when calling window display changed.
     *
     * @param callingWindowInfo The information about the calling window.
     */
    virtual void OnCallingWindowDisplayChanged(const CallingWindowInfo& callingWindowInfo) = 0;
};

/**
 * @class IWindowPidVisibilityChangedListener
 *
 * @brief Listener to observe window visibility that in same pid.
 */
class IWindowPidVisibilityChangedListener : virtual public RefBase {
public:
    /**
     * @brief Notify caller when window style changed.
     *
     * @param info
     */
    virtual void NotifyWindowPidVisibilityChanged(const sptr<WindowPidVisibilityInfo>& info) = 0;
};

/**
 * @class IWindowInfoChangedListener
 *
 * @brief Listener to observe window info.
 */
class IWindowInfoChangedListener : virtual public RefBase {
public:
    IWindowInfoChangedListener() = default;

    virtual ~IWindowInfoChangedListener() = default;

    /**
     * @brief Notify caller when window Info changed.
     *
     * @param windowInfoList
     */
    virtual void OnWindowInfoChanged(const WindowInfoList& windowInfoList) = 0;

    void SetInterestInfo(const std::unordered_set<WindowInfoKey>& interestInfo) { interestInfo_ = interestInfo; }
    const std::unordered_set<WindowInfoKey>& GetInterestInfo() const { return interestInfo_; }
    void AddInterestInfo(WindowInfoKey interestValue) { interestInfo_.insert(interestValue); }
    void SetInterestWindowIds(const std::unordered_set<int32_t>& interestWindowIds)
        { interestWindowIds_ = interestWindowIds; }
    const std::unordered_set<int32_t>& GetInterestWindowIds() const { return interestWindowIds_; }
    void AddInterestWindowId(int32_t interestWindowId) { interestWindowIds_.insert(interestWindowId); }
    void RemoveInterestWindowId(int32_t interestWindowId) { interestWindowIds_.erase(interestWindowId); }

private:
    std::unordered_set<WindowInfoKey> interestInfo_;
    std::unordered_set<int32_t> interestWindowIds_;
};

/*
 * @class IWindowSystemBarPropertyChangedListener
 *
 * @brief Observe the property change of System Bar.
 */
class IWindowSystemBarPropertyChangedListener : virtual public RefBase {
public:
    /**
     * @brief Notify caller when System Bar property changed.
     *
     * @param type Type of System Bar
     * @param systemBarProperty Property of System Bar
     */
    virtual void OnWindowSystemBarPropertyChanged(WindowType type, const SystemBarProperty& systemBarProperty) = 0;
};

/**
 * @class AccessibilityWindowInfo
 *
 * @brief Window info used for Accessibility.
 */
class AccessibilityWindowInfo : public Parcelable {
public:
    /**
     * @brief Default construct of AccessibilityWindowInfo.
     */
    AccessibilityWindowInfo() = default;

    /**
     * @brief Default deconstruct of AccessibilityWindowInfo.
     */
    ~AccessibilityWindowInfo() = default;

    /**
     * @brief Marshalling AccessibilityWindowInfo.
     *
     * @param parcel Package of AccessibilityWindowInfo.
     * @return True means marshall success, false means marshall failed.
     */
    virtual bool Marshalling(Parcel& parcel) const override;

    /**
     * @brief Unmarshalling AccessibilityWindowInfo.
     *
     * @param parcel Package of AccessibilityWindowInfo.
     * @return AccessibilityWindowInfo object.
     */
    static AccessibilityWindowInfo* Unmarshalling(Parcel& parcel);

    int32_t wid_;
    int32_t innerWid_;
    int32_t uiNodeId_;
    Rect windowRect_;
    bool focused_ { false };
    bool isDecorEnable_ { false };
    DisplayId displayId_;
    uint32_t layer_;
    WindowMode mode_;
    WindowType type_;
    float scaleVal_;
    float scaleX_;
    float scaleY_;
    bool isCompatScaleMode_ { false };
    Rect scaleRect_;
    std::string bundleName_;
    std::vector<Rect> touchHotAreas_;
};

/**
 * @class AppUseControlInfo
 *
 * @brief Window info used for AppUseControlInfo.
 */
struct AppUseControlInfo : public Parcelable {
    /**
     * @brief Marshalling AppUseControlInfo.
     *
     * @param parcel Package of AppUseControlInfo.
     * @return True means marshall success, false means marshall failed.
     */
    virtual bool Marshalling(Parcel& parcel) const override
    {
        return parcel.WriteString(bundleName_) &&
               parcel.WriteInt32(appIndex_) &&
               parcel.WriteInt32(persistentId_) &&
               parcel.WriteBool(isNeedControl_) &&
               parcel.WriteBool(isControlRecentOnly_);
    }

    /**
     * @brief Unmarshalling AppUseControlInfo.
     *
     * @param parcel Package of AppUseControlInfo.
     * @return AppUseControlInfo object.
     */
    static AppUseControlInfo* Unmarshalling(Parcel& parcel)
    {
        auto info = new AppUseControlInfo();
        if (!parcel.ReadString(info->bundleName_) ||
            !parcel.ReadInt32(info->appIndex_) ||
            !parcel.ReadInt32(info->persistentId_) ||
            !parcel.ReadBool(info->isNeedControl_) ||
            !parcel.ReadBool(info->isControlRecentOnly_)) {
            delete info;
            return nullptr;
        }
        return info;
    }

    std::string bundleName_ = "";
    int32_t appIndex_ = 0;
    int32_t persistentId_ = INVALID_WINDOW_ID; // greater than 0 means control by id
    bool isNeedControl_ = false;
    bool isControlRecentOnly_ = false;
};

/**
 * @struct AbilityInfoBase
 *
 * @brief ability info.
 */
struct AbilityInfoBase : public Parcelable {
    /**
     * @brief Marshalling AbilityInfoBase.
     *
     * @param parcel Package of AbilityInfoBase.
     * @return True means marshall success, false means marshall failed.
     */
    bool Marshalling(Parcel& parcel) const override
    {
        return parcel.WriteString(bundleName) &&
               parcel.WriteString(moduleName) &&
               parcel.WriteString(abilityName) &&
               parcel.WriteInt32(appIndex);
    }

    /**
     * @brief Unmarshalling AbilityInfoBase.
     *
     * @param parcel Package of AbilityInfoBase.
     * @return AbilityInfoBase object.
     */
    static AbilityInfoBase* Unmarshalling(Parcel& parcel)
    {
        auto info = new AbilityInfoBase();
        if (!parcel.ReadString(info->bundleName) ||
            !parcel.ReadString(info->moduleName) ||
            !parcel.ReadString(info->abilityName) ||
            !parcel.ReadInt32(info->appIndex)) {
            delete info;
            return nullptr;
        }
        return info;
    }

    bool IsValid() const
    {
        return !bundleName.empty() && !moduleName.empty() && !abilityName.empty() && appIndex >= 0;
    }

    std::string ToKeyString() const
    {
        return bundleName + "_" + moduleName + "_" + abilityName + "_" + std::to_string(appIndex);
    }

    std::string bundleName;
    std::string moduleName;
    std::string abilityName;
    int32_t appIndex = 0;
};

/**
 * @class UnreliableWindowInfo
 *
 * @brief Unreliable Window Info.
 */
class UnreliableWindowInfo : public Parcelable {
public:
    /**
     * @brief Default construct of UnreliableWindowInfo.
     */
    UnreliableWindowInfo() = default;

    /**
     * @brief Default deconstruct of UnreliableWindowInfo.
     */
    ~UnreliableWindowInfo() = default;

    /**
     * @brief Marshalling UnreliableWindowInfo.
     *
     * @param parcel Package of UnreliableWindowInfo.
     * @return True means marshall success, false means marshall failed.
     */
    virtual bool Marshalling(Parcel& parcel) const override;

    /**
     * @brief Unmarshalling UnreliableWindowInfo.
     *
     * @param parcel Package of UnreliableWindowInfo.
     * @return UnreliableWindowInfo object.
     */
    static UnreliableWindowInfo* Unmarshalling(Parcel& parcel);

    int32_t windowId_ { 0 };
    Rect windowRect_;
    uint32_t zOrder_ { 0 };
    float floatingScale_ { 1.0f };
    float scaleX_ { 1.0f };
    float scaleY_ { 1.0f };
};

/**
 * @class IWindowUpdateListener
 *
 * @brief Listener to observe window update.
 */
class IWindowUpdateListener : virtual public RefBase {
public:
    /**
     * @brief Notify caller when AccessibilityWindowInfo update.
     *
     * @param infos Window info used for Accessibility.
     * @param type Type for window update.
     */
    virtual void OnWindowUpdate(const std::vector<sptr<AccessibilityWindowInfo>>& infos, WindowUpdateType type) = 0;
};

/**
 * @class IWaterMarkFlagChangedListener
 *
 * @brief Listener to observe water mark flag changed.
 */
class IWaterMarkFlagChangedListener : virtual public RefBase {
public:
    /**
     * @brief Notify caller when water mark flag changed.
     *
     * @param showWaterMark True means show water mark, false means the opposite.
     */
    virtual void OnWaterMarkFlagUpdate(bool showWaterMark) = 0;
};

/**
 * @class IVisibleWindowNumChangedListener
 *
 * @brief Listener to observe visible main window num changed.
 */
class IVisibleWindowNumChangedListener : virtual public RefBase {
public:
    /**
     * @brief Notify caller when visible window num changed
     *
     * @param visibleWindowNum visible window num .
     */
    virtual void OnVisibleWindowNumChange(const std::vector<VisibleWindowNumInfo>& visibleWindowNumInfo) = 0;
};

/**
 * @class ICameraFloatWindowChangedListener
 *
 * @brief Listener to observe camera window changed.
 */
class ICameraFloatWindowChangedListener : virtual public RefBase {
public:
    /**
     * @brief Notify caller when camera window changed.
     *
     * @param accessTokenId Token id of camera window.
     * @param isShowing True means camera is shown, false means the opposite.
     */
    virtual void OnCameraFloatWindowChange(uint32_t accessTokenId, bool isShowing) = 0;
};

/**
 * @class ICameraWindowChangedListener
 *
 * @brief Listener to observe camera window changed.
 */
class ICameraWindowChangedListener : virtual public RefBase {
public:
    /**
     * @brief Notify caller when camera window changed.
     *
     * @param accessTokenId Token id of camera window.
     * @param isShowing True means camera is shown, false means the opposite.
     */
    virtual void OnCameraWindowChange(uint32_t accessTokenId, bool isShowing) = 0;
};

/**
 * @class IDisplayInfoChangedListener
 *
 * @brief Listener to observe display information changed.
 */
class IDisplayInfoChangedListener : virtual public RefBase {
public:
    /**
     * @brief Notify caller when display information changed.
     *
     * @param token token of ability.
     * @param displayId ID of the display where the main window of the ability is located.
     * @param density density of the display where the main window of the ability is located.
     * @param orientation orientation of the display where the main window of the ability is located.
     */
    virtual void OnDisplayInfoChange(const sptr<IRemoteObject>& token,
        DisplayId displayId, float density, DisplayOrientation orientation) = 0;
};

/**
 * @class IPiPStateChangedListener
 *
 * @brief Listener to observe PiP State changed.
 */
class IPiPStateChangedListener : virtual public RefBase {
public:
    /**
     * @brief Notify caller when PiP State changed.
     *
     * @param bundleName the name of the bundle in PiP state changed.
     * @param isForeground the state of the bundle in PiP State.
     */
    virtual void OnPiPStateChanged(const std::string& bundleName, bool isForeground) = 0;
};

/*
 * @class IWindowSupportRotationListener
 *
 * @brief IWindowSupportRotationListener is used to observe the window support rotation change.
 */
class IWindowSupportRotationListener : virtual public RefBase {
public:
    /**
     * @brief Notify caller when window support rotation change
     *
     * @param supportRotationInfo information of support rotation
     *
     */
    virtual void OnSupportRotationChange(const SupportRotationInfo& supportRotationInfo) = 0;
};

class WindowManagerAgent;

/**
 * @class WindowManager
 *
 * @brief WindowManager used to manage window.
 */
class WindowManager : public RefBase {
    WM_DECLARE_SINGLE_INSTANCE_BASE(WindowManager);
    friend class WindowManagerAgent;
    friend class WMSDeathRecipient;
    friend class SSMDeathRecipient;
public:
    static WindowManager& GetInstance(const int32_t userId);
    static WMError RemoveInstanceByUserId(const int32_t userId);

    /**
     * @brief Register WMS connection status changed listener.
     * @attention Callable only by u0 system user. A process only supports successful registration once.
     * When the foundation service restarts, you need to re-register the listener.
     * If you want to re-register, please call UnregisterWMSConnectionChangedListener first.
     *
     * @param listener IWMSConnectionChangedListener.
     * @return WM_OK means register success, others means register failed.
     */
    WMError RegisterWMSConnectionChangedListener(const sptr<IWMSConnectionChangedListener>& listener);

    /**
     * @brief Unregister WMS connection status changed listener.
     * @attention Callable only by u0 system user.
     *
     * @return WM_OK means unregister success, others means unregister failed.
     */
    WMError UnregisterWMSConnectionChangedListener();

    /**
     * @brief Register focus changed listener.
     *
     * @param listener IFocusChangedListener.
     * @return WM_OK means register success, others means register failed.
     */
    WMError RegisterFocusChangedListener(const sptr<IFocusChangedListener>& listener);

    /**
     * @brief Unregister focus changed listener.
     *
     * @param listener IFocusChangedListener.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    WMError UnregisterFocusChangedListener(const sptr<IFocusChangedListener>& listener);

    /**
     * @brief Register window mode listener.
     *
     * @param listener IWindowModeChangedListener.
     * @return WM_OK means register success, others means register failed.
     */
    WMError RegisterWindowModeChangedListener(const sptr<IWindowModeChangedListener>& listener);

    /**
     * @brief Unregister window mode listener.
     *
     * @param listener IWindowModeChangedListener.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    WMError UnregisterWindowModeChangedListener(const sptr<IWindowModeChangedListener>& listener);

    /**
     * @brief Get window mode type.
     *
     * @param void
     * @return WM_OK means get success, others means get failed.
     */
    WMError GetWindowModeType(WindowModeType& windowModeType) const;

    /**
     * @brief Register system bar changed listener.
     *
     * @param listener ISystemBarChangedListener.
     * @return WM_OK means register success, others means register failed.
     */
    WMError RegisterSystemBarChangedListener(const sptr<ISystemBarChangedListener>& listener);

    /**
     * @brief Unregister system bar changed listener.
     *
     * @param listener ISystemBarChangedListener.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    WMError UnregisterSystemBarChangedListener(const sptr<ISystemBarChangedListener>& listener);

    /**
     * @brief Register window updated listener.
     *
     * @param listener IWindowUpdateListener.
     * @return WM_OK means register success, others means register failed.
     */
    WMError RegisterWindowUpdateListener(const sptr<IWindowUpdateListener>& listener);

    /**
     * @brief Unregister window updated listener.
     *
     * @param listener IWindowUpdateListener.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    WMError UnregisterWindowUpdateListener(const sptr<IWindowUpdateListener>& listener);

    /**
     * @brief Register visibility changed listener.
     *
     * @param listener IVisibilityChangedListener.
     * @return WM_OK means register success, others means register failed.
     */
    WMError RegisterVisibilityChangedListener(const sptr<IVisibilityChangedListener>& listener);

    /**
     * @brief Unregister visibility changed listener.
     *
     * @param listener IVisibilityChangedListener.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    WMError UnregisterVisibilityChangedListener(const sptr<IVisibilityChangedListener>& listener);

    /**
     * @brief Register drawingcontent changed listener.
     *
     * @param listener IDrawingContentChangedListener.
     * @return WM_OK means register success, others means register failed.
     */
    WMError RegisterDrawingContentChangedListener(const sptr<IDrawingContentChangedListener>& listener);

    /**
     * @brief Unregister drawingcontent changed listener.
     *
     * @param listener IDrawingContentChangedListener.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    WMError UnregisterDrawingContentChangedListener(const sptr<IDrawingContentChangedListener>& listener);

    /**
     * @brief Register camera float window changed listener.
     *
     * @param listener ICameraFloatWindowChangedListener.
     * @return WM_OK means register success, others means register failed.
     */
    WMError RegisterCameraFloatWindowChangedListener(const sptr<ICameraFloatWindowChangedListener>& listener);

    /**
     * @brief Unregister camera float window changed listener.
     *
     * @param listener ICameraFloatWindowChangedListener.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    WMError UnregisterCameraFloatWindowChangedListener(const sptr<ICameraFloatWindowChangedListener>& listener);

    /**
     * @brief Register water mark flag changed listener.
     *
     * @param listener IWaterMarkFlagChangedListener.
     * @return WM_OK means register success, others means register failed.
     */
    WMError RegisterWaterMarkFlagChangedListener(const sptr<IWaterMarkFlagChangedListener>& listener);

    /**
     * @brief Unregister water mark flag changed listener.
     *
     * @param listener IWaterMarkFlagChangedListener.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    WMError UnregisterWaterMarkFlagChangedListener(const sptr<IWaterMarkFlagChangedListener>& listener);

    /**
     * @brief Register gesture navigation enabled changed listener.
     *
     * @param listener IGestureNavigationEnabledChangedListener.
     * @return WM_OK means register success, others means register failed.
     */
    WMError RegisterGestureNavigationEnabledChangedListener(
        const sptr<IGestureNavigationEnabledChangedListener>& listener);

    /**
     * @brief Unregister gesture navigation enabled changed listener.
     *
     * @param listener IGestureNavigationEnabledChangedListener.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    WMError UnregisterGestureNavigationEnabledChangedListener(
        const sptr<IGestureNavigationEnabledChangedListener>& listener);

    /**
     * @brief register display information changed listener.
     *
     * @param token token of ability.
     * @param listener IDisplayInfoChangedListener.
     * @return WM_OK means register success, others means register failed.
     */
    WMError RegisterDisplayInfoChangedListener(const sptr<IRemoteObject>& token,
        const sptr<IDisplayInfoChangedListener>& listener);

    /**
     * @brief unregister display info changed listener.Before the ability is destroyed, the
     * UnregisterDisplayInfoChangedListener interface must be invoked.
     * Otherwise, the sptr token may be destroyed abnormally.
     *
     * @param token token of ability.
     * @param listener IDisplayInfoChangedListener.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    WMError UnregisterDisplayInfoChangedListener(const sptr<IRemoteObject>& token,
        const sptr<IDisplayInfoChangedListener>& listener);

    /**
     * @brief Register window in same pid visibility changed listener.
     * @caller SA
     * @permission SA permission
     *
     * @param listener IWindowPidVisibilityChangedListener.
     * @return WM_OK means register success, others means register failed.
     */
    WMError RegisterWindowPidVisibilityChangedListener(const sptr<IWindowPidVisibilityChangedListener>& listener);

    /**
     * @brief Unregister window in same pid visibility changed listener.
     *
     * @param listener IWindowPidVisibilityChangedListener.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    WMError UnregisterWindowPidVisibilityChangedListener(const sptr<IWindowPidVisibilityChangedListener>& listener);

    /*
     * @brief Register window System Bar property changed Listener.
     *
     * @param listener IWindowSystemBarPropertyChangedListener.
     * @return WM_OK means register success, others means register failed.
     */
    WMError RegisterWindowSystemBarPropertyChangedListener(
        const sptr<IWindowSystemBarPropertyChangedListener>& listener);

    /*
     * @brief Unregister window System Bar property changed Listener.
     *
     * @param listener IWindowSystemBarPropertyChangedListener.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    WMError UnregisterWindowSystemBarPropertyChangedListener(
        const sptr<IWindowSystemBarPropertyChangedListener>& listener);

    /*
     * @brief Notify window System Bar property changed.
     *
     * @param type Type of System Bar
     * @param systemBarProperty Property of System Bar
     */
    void NotifyWindowSystemBarPropertyChange(WindowType type, const SystemBarProperty& systemBarProperty);

    /**
     * @brief notify display information change.
     *
     * @param token ability token.
     * @param displayid ID of the display where the main window of the ability is located
     * @param density density of the display where the main window of the ability is located.
     * @param orientation orientation of the display where the main window of the ability is located.
     * @return WM_OK means notify success, others means notify failed.
    */
    WMError NotifyDisplayInfoChange(const sptr<IRemoteObject>& token, DisplayId displayId,
        float density, DisplayOrientation orientation);

    /**
     * @brief notify window info change.
     *
     * @param flags mark the changed value.
     * @param windowInfoList the changed window info list.
     * @return WM_OK means notify success, others means notify failed.
     */
    void NotifyWindowPropertyChange(uint32_t propertyDirtyFlags, const WindowInfoList& windowInfoList);

    /**
     * @brief Minimize all app window.
     *
     * @param displayId Display id.
     * @param excludeWindowId Exclude window id.
     * @return WM_OK means minimize success, others means minimize failed.
     */
    WMError MinimizeAllAppWindows(DisplayId displayId, int32_t excludeWindowId = 0);

    /**
     * @brief Toggle all app windows to the foreground.
     *
     * @return WM_OK means toggle success, others means toggle failed.
     */
    WMError ToggleShownStateForAllAppWindows();

    /**
     * @brief Set window layout mode.
     *
     * @param mode Window layout mode.
     * @return WM_OK means set success, others means set failed.
     */
    WMError SetWindowLayoutMode(WindowLayoutMode mode);

    /**
     * @brief Global coordinate to relative coordinate conversion in Extension
     *
     * @param rect Rect relative to the default display
     * @param newRect Rect relative to the current display
     * @param newDisplayId Current displayID
     * @return WM_OK means converted, others means not converted.
     */
    WMError ConvertToRelativeCoordinateExtended(const Rect& rect, Rect& newRect, DisplayId& newDisplayId);

    /**
     * @brief Get accessibility window info.
     *
     * @param infos WindowInfos used for Accessibility.
     * @return WM_OK means get success, others means get failed.
     */
    WMError GetAccessibilityWindowInfo(std::vector<sptr<AccessibilityWindowInfo>>& infos) const;

    /**
     * @brief Get unreliable window info.
     *
     * @param infos Unreliable Window Info.
     * @return WM_OK means get success, others means get failed.
     */
    WMError GetUnreliableWindowInfo(int32_t windowId,
        std::vector<sptr<UnreliableWindowInfo>>& infos) const;

    /**
     * @brief List window info.
     *
     * @param windowInfoOption Option for selecting window info.
     * @param infos Window info.
     * @return WM_OK means get success, others means get failed.
     */
    WMError ListWindowInfo(const WindowInfoOption& windowInfoOption, std::vector<sptr<WindowInfo>>& infos) const;

    /**
     * @brief Get window layout info.
     *
     * @param displayId DisplayId of which display to get window layout infos.
     * @param infos Window layout infos.
     * @return WM_OK means get success, others means get failed.
     */
    WMError GetAllWindowLayoutInfo(DisplayId displayId, std::vector<sptr<WindowLayoutInfo>>& infos) const;

    /**
     * @brief Get global window mode.
     *
     * @param displayId DisplayId of which display to get window mode, DISPLAY_ID_INVALID means all displays.
     * @param globalWinMode Global window mode flag of specified display or all displays.
     * @return WM_OK means get success, others means get failed.
     */
    WMError GetGlobalWindowMode(DisplayId displayId, GlobalWindowMode& globalWinMode) const;

    /**
     * @brief Get main window info.
     *
     * @param infos Main Window infos.
     * @return WM_OK means get success, others means get failed.
     */
    WMError GetAllMainWindowInfo(std::vector<sptr<MainWindowInfo>>& infos) const;

    /**
     * @brief Get main window snap shot.
     *
     * @param windowIds Window id which want to get.
     * @param config Snapshot configuration.
     * @param callback callback.
     * @return WM_OK means get success, others means get failed.
     */
    WMError GetMainWindowSnapshot(const std::vector<int32_t>& windowIds, const WindowSnapshotConfiguration& config,
        const sptr<IRemoteObject>& callback) const;

    /**
     * @brief Get the name of the top page.
     *
     * @param windowId Window id which want to get.
     * @param topNavDestName The top page name of specified window.
     * @return WM_OK means get success, others means get failed.
     */
    WMError GetTopNavDestinationName(int32_t windowId, std::string& topNavDestName) const;

    /**
     * @brief Set watermark for app.
     *
     * @param pixelMap the watermark image to set, nullptr means cancel watermark.
     * @return WM_OK means get success, others means get failed.
     */
    WMError SetWatermarkImageForApp(const std::shared_ptr<Media::PixelMap>& pixelMap);

    /**
     * @brief Get visibility window info.
     *
     * @param infos Visible window infos
     * @return WM_OK means get success, others means get failed.
     */
    WMError GetVisibilityWindowInfo(std::vector<sptr<WindowVisibilityInfo>>& infos) const;

    /**
     * @brief Set gesture navigation enabled.
     *
     * @param enable True means set gesture on, false means set gesture off.
     * @return WM_OK means set success, others means set failed.
     */
    WMError SetGestureNavigationEnabled(bool enable) const;

    /**
     * @brief Get focus window.
     *
     * @param focusInfo Focus window info.
     * @return FocusChangeInfo object about focus window.
     */
    void GetFocusWindowInfo(FocusChangeInfo& focusInfo, DisplayId displayId = DEFAULT_DISPLAY_ID);

    /**
     * @brief Get focus window info.
     *
     * @param focusInfo Focus window info.
     * @param abilityToken Ability token.
     * @return FocusChangeInfo object about focus window.
     */
    void GetFocusWindowInfoByAbilityToken(FocusChangeInfo& focusInfo, const sptr<IRemoteObject>& abilityToken);

    /**
     * @brief Dump all session info
     *
     * @param infos session infos
     * @return WM_OK means set success, others means set failed.
     */
    WMError DumpSessionAll(std::vector<std::string>& infos);

    /**
     * @brief Dump session info with id
     *
     * @param infos session infos
     * @return WM_OK means set success, others means set failed.
     */
    WMError DumpSessionWithId(int32_t persistentId, std::vector<std::string>& infos);

    /**
     * @brief Get uiContent remote object
     *
     * @param windowId windowId
     * @param uiContentRemoteObj uiContentRemoteObj
     * @return WM_OK if successfully retrieved uiContentRemoteObj
     */
    WMError GetUIContentRemoteObj(int32_t windowId, sptr<IRemoteObject>& uiContentRemoteObj);

    /**
     * @brief Get uiContent remote object for root scene
     *
     * @param displayId display id
     * @param uiContentRemoteObj remote uiContent object
     * @return WM_OK means set success, others means set failed.
     */
    WMError GetRootUIContentRemoteObj(DisplayId displayId, sptr<IRemoteObject>& uiContentRemoteObj);

    /**
     * @brief raise window to top by windowId
     *
     * @param persistentId this window to raise
     * @return WM_OK if raise success
     */
    WMError RaiseWindowToTop(int32_t persistentId);

    /**
     * @brief notify window extension visibility change
     *
     * @param pid process id
     * @param uid user id
     * @param visible visibility
     * @return WM_OK means notify success, others means notify failed.
     */
    WMError NotifyWindowExtensionVisibilityChange(int32_t pid, int32_t uid, bool visible);

    /**
     * @brief Shift window focus within the same application. Only main window and subwindow.
     *
     * @param sourcePersistentId Window id which the focus shift from
     * @param targetPersistentId Window id which the focus shift to
     * @return WM_OK means shift window focus success, others means failed.
     */
    WMError ShiftAppWindowFocus(int32_t sourcePersistentId, int32_t targetPersistentId);

    /**
     * @brief Set zIndex of the specific system window with window type.
     *
     * @param windowType Window type of the specific system window
     * @param zIndex  The value of ZIndex
     * @return WM_OK means set zIndex success, others means failed.
     */
    WMError SetSpecificSystemWindowZIndex(WindowType windowType, int32_t zIndex);

    /**
     * @brief Set start window background color.
     *
     * @param moduleName Module name that needs to be set
     * @param abilityName Ability name that needs to be set
     * @param color Color metrics
     * @return WM_OK means set start window background color success, others means failed.
     */
    WMError SetStartWindowBackgroundColor(
        const std::string& moduleName, const std::string& abilityName, uint32_t color);

    /**
     * @brief Get snapshot by window id.
     *
     * @param windowId Window id which want to snapshot.
     * @param pixelMap Snapshot output pixel map.
     * @return WM_OK means get snapshot success, others means failed.
     */
    WMError GetSnapshotByWindowId(int32_t windowId, std::shared_ptr<Media::PixelMap>& pixelMap);

    /**
     * @brief Register visible main window num changed listener.
     *
     * @param listener IVisibleWindowNumChangedListener.
     * @return WM_OK means register success, others means register failed.
     */
    WMError RegisterVisibleWindowNumChangedListener(const sptr<IVisibleWindowNumChangedListener>& listener);

    /**
     * @brief Unregister visible main window num changed listener.
     *
     * @param listener IVisibleWindowNumChangedListener.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    WMError UnregisterVisibleWindowNumChangedListener(const sptr<IVisibleWindowNumChangedListener>& listener);

    /**
     * @brief Register WindowStyle changed listener.
     *
     * @param listener IWindowStyleChangedListener
     * @return WM_OK means register success, others means unregister failed.
     */
    WMError RegisterWindowStyleChangedListener(const sptr<IWindowStyleChangedListener>& listener);

    /**
     * @brief Unregister WindowStyle changed listener.
     *
     * @param listener IWindowStyleChangedListener
     * @return WM_OK means unregister success, others means unregister failed.
     */
    WMError UnregisterWindowStyleChangedListener(const sptr<IWindowStyleChangedListener>& listener);

    /**
     * @brief Register a listener to detect display changes for the keyboard calling window.
     *
     * @param listener IKeyboardCallingWindowDisplayChangedListener
     * @return WM_OK means register success, others means unregister failed.
     */
    WMError RegisterCallingWindowDisplayChangedListener(
        const sptr<IKeyboardCallingWindowDisplayChangedListener>& listener)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    
    /**
     * @brief Unregister the listener that detects display changes for the keyboard calling window.
     *
     * @param listener IKeyboardCallingWindowDisplayChangedListener
     * @return WM_OK means unregister success, others means unregister failed.
     */
    WMError UnregisterCallingWindowDisplayChangedListener(
        const sptr<IKeyboardCallingWindowDisplayChangedListener>& listener)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Skip Snapshot for app process.
     *
     * @param pid process id
     * @param skip True means skip, false means not skip.
     * @return WM_OK means snapshot skip success, others means snapshot skip failed.
     */
    WMError SkipSnapshotForAppProcess(int32_t pid, bool skip);

    /**
     * @brief Get window style type.
     *
     * @param windowStyleType WindowType
     * @return @return WM_OK means get window style success, others means failed.
     */
    WindowStyleType GetWindowStyleType();

    /**
     * @brief set process watermark.
     *
     * @param pid pid
     * @param watermarkName watermark picture name
     * @param isEnabled add or remove
     * @return WM_OK means set process watermark success, others means failed.
     */
    WMError SetProcessWatermark(int32_t pid, const std::string& watermarkName, bool isEnabled);

    /**
     * @brief Get window ids by coordinate.
     *
     * @param displayId display id
     * @param windowNumber indicates the number of query windows
     * @param x x-coordinate of the window
     * @param y y-coordinate of the window
     * @param windowIds array of window id
     * @return WM_OK means get success, others means get failed.
     */
    WMError GetWindowIdsByCoordinate(DisplayId displayId, int32_t windowNumber,
        int32_t x, int32_t y, std::vector<int32_t>& windowIds) const;

    /**
     * @brief Update screen lock status for app.
     *
     * @param bundleName BundleName of specific app
     * @param isRelease True means screen lock, false means reLock screen lock
     * @return WM_OK means update success, others means failed.
     */
    WMError UpdateScreenLockStatusForApp(const std::string& bundleName, bool isRelease);

    /**
     * @brief Get displayId by windowId.
     *
     * @param windowIds list of window ids that need to get screen ids
     * @param windowDisplayIdMap map of windows and displayIds
     * @return WM_OK means get success, others means failed.
     */
    WMError GetDisplayIdByWindowId(const std::vector<uint64_t>& windowIds,
        std::unordered_map<uint64_t, DisplayId>& windowDisplayIdMap);

    /**
     * @brief Set global drag resize type.
     * this priority is highest.
     *
     * @param dragResizeType global drag resize type to set
     * @return WM_OK means set success, others means failed.
     */
    WMError SetGlobalDragResizeType(DragResizeType dragResizeType);

    /**
     * @brief Get global drag resize type.
     * if it is RESIZE_TYPE_UNDEFINED, return default value.
     *
     * @param dragResizeType global drag resize type to get
     * @return WM_OK means get success, others means failed.
     */
    WMError GetGlobalDragResizeType(DragResizeType& dragResizeType);

    /**
     * @brief Set drag resize type of specific app.
     * only when global value is RESIZE_TYPE_UNDEFINED, this value take effect.
     *
     * @param bundleName bundleName of specific app
     * @param dragResizeType drag resize type to set
     * @return WM_OK means set success, others means failed.
     */
    WMError SetAppDragResizeType(const std::string& bundleName, DragResizeType dragResizeType);

    /**
     * @brief Get drag resize type of specific app.
     * effective order:
     *  1. global value
     *  2. app value
     *  3. default value
     *
     * @param bundleName bundleName of specific app
     * @param dragResizeType drag resize type to get
     * @return WM_OK means get success, others means failed.
     */
    WMError GetAppDragResizeType(const std::string& bundleName, DragResizeType& dragResizeType);

    /**
     * @brief Set drag key frame type of specific app.
     * effective order:
     *  1. resize when drag
     *  2. key frame
     *  3. default value
     *
     * @param bundleName bundleName of specific app
     * @param keyFramePolicy param of key frame
     * @return WM_OK means set success, others means failed.
     */
    WMError SetAppKeyFramePolicy(const std::string& bundleName, const KeyFramePolicy& keyFramePolicy);

    /**
     * @brief Shift window pointer event within the same application. Only main window and subwindow.
     *
     * @param sourceWindowId Window id which the pointer event shift from
     * @param targetWindowId Window id which the pointer event shift to
     * @param fingerId finger id of the event to be shift
     * @return WM_OK means shift window pointer event success, others means failed.
     */
    WMError ShiftAppWindowPointerEvent(int32_t sourceWindowId, int32_t targetWindowId, int32_t fingerId = -1);

    /**
     * @brief Notify screenshot event.
     *
     * @param type screenshot event type.
     * @return WM_OK means set success, others means set failed.
     */
    WMError NotifyScreenshotEvent(ScreenshotEventType type);

    /**
     * @brief Request focus.
     *
     * @param persistentId previous window id
     * @param isFocused true if request focus, otherwise false, default is true
     * @param byForeground true if by foreground, otherwise false, default is true
     * @param reason the reason for requesting focus, default is SA_REQUEST
     * @return WM_OK means request focus success, others means failed.
     */
    WMError RequestFocus(int32_t persistentId, bool isFocused = true,
        bool byForeground = true, WindowFocusChangeReason reason = WindowFocusChangeReason::SA_REQUEST);

    /**
     * @brief Minimize window within the vector of windowid, Only main window.
     *
     * @param WindowId window id which to minimize
     * @return WM_OK means window minimize event success, others means failed.
     */
    WMError MinimizeByWindowId(const std::vector<int32_t>& windowIds);

    /**
     * @brief Set foreground window number. Only main window. Only support freeMultiWindow.
     *
     * @param windowNum foreground window number
     * @return WM_OK means set success, others means failed.
     */
    WMError SetForegroundWindowNum(uint32_t windowNum);

    /**
     * @brief Register window info change callback.
     *
     * @param observedInfo Property which to observe.
     * @param listener Listener to observe window info.
     * @return WM_OK means register success, others means failed.
     */
    WMError RegisterWindowInfoChangeCallback(const std::unordered_set<WindowInfoKey>& observedInfo,
        const sptr<IWindowInfoChangedListener>& listener);
    
    /**
     * @brief Unregister window info change callback.
     *
     * @param observedInfo Property which to observe.
     * @param listener Listener to observe window info.
     * @return WM_OK means unregister success, others means failed.
     */
    WMError UnregisterWindowInfoChangeCallback(const std::unordered_set<WindowInfoKey>& observedInfo,
        const sptr<IWindowInfoChangedListener>& listener);
    
    /**
     * @brief Whether new requested window needs hook.
     *
     * @param isModuleAbilityHookEnd Whether this module finished ability hook.
     * @param moduleName Indicates the hooked moduleName.
     * @return bool means the module finished ability hook or not.
     */
    bool IsModuleHookOff(bool isModuleAbilityHookEnd, const std::string& moduleName);

    /**
     * @brief set the animation property and parameter to the corresponding window.
     *
     * @param windowId target window id.
     * @param animationProperty the property of animation.
     * @param animationOption the option of animation.
     * @return WM_OK means set success, others means failed.
     */
    WMError AnimateTo(int32_t windowId, WindowAnimationProperty animationProperty,
        WindowAnimationOption animationOption);

    /**
     * @brief Register window lifecycle status changed callback.
     * @param listener IWindowLifeCycleListener.
     * @return WM_OK means register success, others means register failed.
     */
    WMError RegisterWindowLifeCycleCallback(const sptr<IWindowLifeCycleListener>& listener);

    /**
     * @brief Unregister window lifecycle status changed callback.
     * @param listener IWindowLifeCycleListener.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    WMError UnregisterWindowLifeCycleCallback(const sptr<IWindowLifeCycleListener>& listener);

    /**
     * @brief Register window support rotation change listener.
     *
     * @param listener IWindowSupportRotationListener.
     * @return WM_OK means register success, others means register failed.
     */
    WMError RegisterWindowSupportRotationListener(const sptr<IWindowSupportRotationListener>& listener);

    /**
     * @brief Unregister window support rotation change listener.
     *
     * @param listener IWindowSupportRotationListener.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    WMError UnregisterWindowSupportRotationListener(const sptr<IWindowSupportRotationListener>& listener);

    /*
     * notify support rotation change to listener
     */
    void NotifySupportRotationChange(const SupportRotationInfo& supportRotationInfo);

    /**
     * @brief Register get js window callback.
     * @param getJSWindowFunc get js window obj callback.
     */
    void RegisterGetJSWindowCallback(GetJSWindowObjFunc&& getJSWindowFunc);

    /**
     * @brief notify window destroyed.
     * @param lifeCycleInfo window lifecycle info.
     */
    void NotifyWMSWindowDestroyed(const WindowLifeCycleInfo& lifeCycleInfo);

    /**
     * @brief Add BundleNames to the list that will hide on virtual screen.
     * @param bundleNames BundleNames that need to add.
     * @param privacyWindowTags Tags of privacy window.
     * @return WM_OK means add success, others means add failed.
     */
    WMError AddSessionBlackList(
        const std::unordered_set<std::string>& bundleNames, const std::unordered_set<std::string>& privacyWindowTags);

    /**
     * @brief Remove BundleNames from the list that will hide on virtual screen.
     * @param bundleNames BundleNames that need to remove.
     * @param privacyWindowTags Tags of privacy window.
     * @return WM_OK means remove success, others means remove failed.
     */
    WMError RemoveSessionBlackList(
        const std::unordered_set<std::string>& bundleNames, const std::unordered_set<std::string>& privacyWindowTags);

    /**
     * @brief Update outline style of window.
     * @param remoteObject Remote object who needs to update outline.
     * @param outlineParams The outline params.
     * @return WM_OK means update success, others means update failed.
     */
    WMError UpdateOutline(const sptr<IRemoteObject>& remoteObject, const OutlineParams& outlineParams);

private:
    /**
     * multi user and multi screen
     */
    friend class sptr<WindowManager>;
    WindowManager(const int32_t userId = INVALID_USER_ID);
    ~WindowManager() override;

    int32_t userId_;
    static std::unordered_map<int32_t, sptr<WindowManager>> windowManagerMap_;
    static std::mutex windowManagerMapMutex_;

    std::recursive_mutex mutex_;
    class Impl;
    std::unique_ptr<Impl> pImpl_;
    std::unordered_set<std::string> isModuleHookOffSet_;
    std::unordered_map<WindowInfoKey, uint32_t> interestInfoMap_;
    GetJSWindowObjFunc getJSWindowObjFunc_;

    /**
     * Update window outline.
     */
    sptr<IRemoteObject> outlineRemoteObject_ = nullptr;
    OutlineParams outlineParams_;
    bool isOutlineRecoverRegistered_ = false;
    WMError CheckOutlineParams(const sptr<IRemoteObject>& remoteObject, const OutlineParams& outlineParams);
    
    void OnWMSConnectionChanged(int32_t userId, int32_t screenId, bool isConnected) const;
    void UpdateFocusStatus(uint32_t windowId, const sptr<IRemoteObject>& abilityToken, WindowType windowType,
        DisplayId displayId, bool focused) const;
    void UpdateFocusChangeInfo(const sptr<FocusChangeInfo>& focusChangeInfo, bool focused) const;
    void UpdateWindowModeTypeInfo(WindowModeType type) const;
    void UpdateSystemBarRegionTints(DisplayId displayId, const SystemBarRegionTints& tints) const;
    void NotifyAccessibilityWindowInfo(const std::vector<sptr<AccessibilityWindowInfo>>& infos,
        WindowUpdateType type) const;
    void UpdateWindowVisibilityInfo(
        const std::vector<sptr<WindowVisibilityInfo>>& windowVisibilityInfos) const;
    void UpdateWindowDrawingContentInfo(
        const std::vector<sptr<WindowDrawingContentInfo>>& windowDrawingContentInfos) const;
    void UpdateCameraFloatWindowStatus(uint32_t accessTokenId, bool isShowing) const;
    void NotifyWaterMarkFlagChangedResult(bool showWaterMark) const;
    void NotifyGestureNavigationEnabledResult(bool enable) const;
    void UpdateVisibleWindowNum(const std::vector<VisibleWindowNumInfo>& visibleWindowNumInfo);
    WMError NotifyWindowStyleChange(WindowStyleType type);
    void NotifyWindowPidVisibilityChanged(const sptr<WindowPidVisibilityInfo>& info) const;
    WMError ProcessRegisterWindowInfoChangeCallback(WindowInfoKey observedInfo,
        const sptr<IWindowInfoChangedListener>& listener);
    WMError ProcessUnregisterWindowInfoChangeCallback(WindowInfoKey observedInfo,
        const sptr<IWindowInfoChangedListener>& listener);
    WMError RegisterVisibilityStateChangedListener(const sptr<IWindowInfoChangedListener>& listener);
    WMError UnregisterVisibilityStateChangedListener(const sptr<IWindowInfoChangedListener>& listener);
    WMError RegisterDisplayIdChangedListener(const sptr<IWindowInfoChangedListener>& listener);
    WMError UnregisterDisplayIdChangedListener(const sptr<IWindowInfoChangedListener>& listener);
    WMError RegisterRectChangedListener(const sptr<IWindowInfoChangedListener>& listener);
    WMError UnregisterRectChangedListener(const sptr<IWindowInfoChangedListener>& listener);
    WMError RegisterGlobalRectChangedListener(const sptr<IWindowInfoChangedListener>& listener);
    WMError UnregisterGlobalRectChangedListener(const sptr<IWindowInfoChangedListener>& listener);
    WMError RegisterWindowModeChangedListenerForPropertyChange(const sptr<IWindowInfoChangedListener>& listener);
    WMError UnregisterWindowModeChangedListenerForPropertyChange(const sptr<IWindowInfoChangedListener>& listener);
    WMError RegisterFloatingScaleChangedListener(const sptr<IWindowInfoChangedListener>& listener);
    WMError UnregisterFloatingScaleChangedListener(const sptr<IWindowInfoChangedListener>& listener);
    WMError RegisterMidSceneChangedListener(const sptr<IWindowInfoChangedListener>& listener);
    WMError UnregisterMidSceneChangedListener(const sptr<IWindowInfoChangedListener>& listener);
    void SetIsModuleHookOffToSet(const std::string& moduleName);
    bool GetIsModuleHookOffFromSet(const std::string& moduleName);

    // For fault agent re-register.
    void ActiveFaultAgentReregister(const WindowManagerAgentType type,
        const sptr<WindowManagerAgent>& agent, WMError& ret);
};
} // namespace Rosen
} // namespace OHOS

#endif // OHOS_ROSEN_WINDOW_MANAGER_H
