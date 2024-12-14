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

#include <memory>
#include <mutex>
#include <refbase.h>
#include <vector>
#include <iremote_object.h>
#include "wm_single_instance.h"
#include "wm_common.h"
#include "dm_common.h"
#include "focus_change_info.h"
#include "window_visibility_info.h"
#include "window_drawing_content_info.h"
#include "window.h"

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

struct VisibleWindowNumInfo {
    uint32_t displayId;
    uint32_t visibleWindowNum;
};

struct WindowSnapshotDataPack {
    std::shared_ptr<Media::PixelMap> pixelMap = nullptr;
    WMError result = WMError::WM_OK;
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
               parcel.WriteBool(isNeedControl_);
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
            !parcel.ReadBool(info->isNeedControl_)) {
            delete info;
            return nullptr;
        }
        return info;
    }

    std::string bundleName_ = "";
    int32_t appIndex_ = 0;
    bool isNeedControl_ = false;
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

/**
 * @class WindowManager
 *
 * @brief WindowManager used to manage window.
 */
class WindowManager {
WM_DECLARE_SINGLE_INSTANCE_BASE(WindowManager);
friend class WindowManagerAgent;
friend class WMSDeathRecipient;
friend class SSMDeathRecipient;
public:
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
     * @brief Minimize all app window.
     *
     * @param displayId Display id.
     * @return WM_OK means minimize success, others means minimize failed.
     */
    WMError MinimizeAllAppWindows(DisplayId displayId);
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
     * @brief Get visibility window info.
     *
     * @param infos Visible window infos
     * @return WM_OK means get success, others means get failed.
     */
    WMError GetVisibilityWindowInfo(std::vector<sptr<WindowVisibilityInfo>>& infos) const;
    /**
     * @brief Set gesture navigaion enabled.
     *
     * @param enable True means set gesture on, false means set gesture off.
     * @return WM_OK means set success, others means set failed.
     */
    WMError SetGestureNavigaionEnabled(bool enable) const;

    /**
     * @brief Get focus window.
     *
     * @param focusInfo Focus window info.
     * @return FocusChangeInfo object about focus window.
     */
    void GetFocusWindowInfo(FocusChangeInfo& focusInfo);

    /**
     * @brief Dump all session info
     *
     * @param infos session infos
     * @return WM_OK means set success, others means set failed.
     */
    WMError DumpSessionAll(std::vector<std::string> &infos);

    /**
     * @brief Dump session info with id
     *
     * @param infos session infos
     * @return WM_OK means set success, others means set failed.
     */
    WMError DumpSessionWithId(int32_t persistentId, std::vector<std::string> &infos);

    /**
     * @brief Get uiContent remote object
     *
     * @param windowId windowId
     * @param uiContentRemoteObj uiContentRemoteObj
     * @return WM_OK if successfully retrieved uiContentRemoteObj
     */
    WMError GetUIContentRemoteObj(int32_t windowId, sptr<IRemoteObject>& uiContentRemoteObj);

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
     * @brief Get window style type.
     *
     * @param windowStyleType WindowType
     * @return @return WM_OK means get window style success, others means failed.
     */
    WindowStyleType GetWindowStyleType();

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
     * @brief Release screen lock of foreground sessions.
     *
     * @return WM_OK means release success, others means failed.
     */
    WMError ReleaseForegroundSessionScreenLock();

    /**
     * @brief Get displayId by windowId.
     *
     * @param windowIds list of window ids that need to get screen ids
     * @param windowDisplayIdMap map of windows and displayIds
     * @return WM_OK means get success, others means failed.
     */
    WMError GetDisplayIdByWindowId(const std::vector<uint64_t>& windowIds,
        std::unordered_map<uint64_t, DisplayId>& windowDisplayIdMap);

private:
    WindowManager();
    ~WindowManager();
    std::recursive_mutex mutex_;
    class Impl;
    std::unique_ptr<Impl> pImpl_;
    bool destroyed_ = false;

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
};
} // namespace Rosen
} // namespace OHOS

#endif // OHOS_ROSEN_WINDOW_MANAGER_H
