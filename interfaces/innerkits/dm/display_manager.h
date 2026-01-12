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

#ifndef FOUNDATION_DM_DISPLAY_MANAGER_H
#define FOUNDATION_DM_DISPLAY_MANAGER_H

#include <vector>
#include <mutex>
#include <ipc_skeleton.h>
#include <pixel_map.h>
#include <set>

#include "display.h"
#include "dm_common.h"
#include "fold_screen_info.h"
#include "wm_single_instance.h"
#include "screenshot_info.h"
#include "display_change_info.h"

namespace OHOS::Rosen {
/**
 * @brief snapShot config
 */
struct SnapShotConfig {
    DisplayId displayId_ = DISPLAY_ID_INVALID;
    Media::Size imageSize_;
    Media::Rect imageRect_;
    int rotation_;
    bool isCaptureFullOfScreen_ = false;
};

class DisplayManager {
WM_DECLARE_SINGLE_INSTANCE_BASE(DisplayManager);
friend class DMSDeathRecipient;
public:
    class IDisplayListener : public virtual RefBase {
    public:
        /**
         * @brief Notify when a new display is created.
         */
        virtual void OnCreate(DisplayId) = 0;

        /**
         * @brief Notify when the display is destroyed.
         */
        virtual void OnDestroy(DisplayId) = 0;

        /**
         * @brief Notify when the state of a display changes
         */
        virtual void OnChange(DisplayId) = 0;
    };

    class IDisplayAttributeListener : public virtual RefBase {
    public:
        /**
         * @brief Notify when an attribute of a display changed.
         */
        virtual void OnAttributeChange(DisplayId displayId, const std::vector<std::string>& attributes) = 0;
    };

    class IScreenshotListener : public virtual RefBase {
    public:
        /**
         * @brief Notify when a screenshot event occurs.
         *
         * @param info Screenshot info.
         */
        virtual void OnScreenshot([[maybe_unused]]const ScreenshotInfo info) {}
    };

    class IPrivateWindowListener : public virtual RefBase {
    public:
        /**
         * @brief Monitor whether the existence of privacy window has changed.
         *
         * @param hasPrivate True means the display has private window, false means the opposite.
         */
        virtual void OnPrivateWindow([[maybe_unused]]bool hasPrivate) {}
    };

    class IPrivateWindowListChangeListener : public virtual RefBase {
    public:
        /**
         * @brief Monitor whether the existence of privacy window list has changed.
         *
         * @param displayId Id of the target display.
         *
         * @param privacyWindowList privacywindow bundlename list of the target display.
         */
        virtual void OnPrivateWindowListChange([[maybe_unused]]DisplayId displayId,
            [[maybe_unused]]std::vector<std::string> privacyWindowList) {}

        /**
         * @brief only for UT to compare privacy window list.
         *
         * @param callback callback.
         */
        virtual void setCallback([[maybe_unused]]std::function<void(std::vector<std::string>)> callback) {}
    private:
        std::function<void(std::vector<std::string>)> callback_;
    };

    class IFoldStatusListener : public virtual RefBase {
    public:
        /**
         * @brief Notify listeners when screen fold status changed.
         *
         * @param foldStatus Screen foldStatus.
         */
        virtual void OnFoldStatusChanged([[maybe_unused]]FoldStatus foldStatus) {}
    };

    class IFoldAngleListener : public virtual RefBase {
    public:
        /**
         * @brief Notify listeners when screen fold angles changed.
         *
         * @param foldAngles Screen fold angles array.
         */
        virtual void OnFoldAngleChanged([[maybe_unused]]std::vector<float> foldAngles) {}
    };

    class ICaptureStatusListener : public virtual RefBase {
    public:
        /**
         * @brief Notify listeners when screen capture status changed.
         *
         * @param isCapture Screen capture status.
         */
        virtual void OnCaptureStatusChanged([[maybe_unused]]bool isCapture) {}
    };

    class IDisplayUpdateListener : public virtual RefBase {
    public:
        /**
         * @brief Notify listeners when session change display.
         *
         * @param infos DisplayChangeInfo.
         */
        virtual void OnDisplayUpdate([[maybe_unused]]const sptr<DisplayChangeInfo>& info) {}
    };

    class IDisplayModeListener : public virtual RefBase {
    public:
        /**
         * @brief Notify listeners when display mode changed.
         *
         * @param displayMode DisplayMode.
         */
        virtual void OnDisplayModeChanged([[maybe_unused]]FoldDisplayMode displayMode) {}
    };

    class IScreenMagneticStateListener : public virtual RefBase {
    public:
        /**
         * @brief Notify listeners when screen magnetic state changed.
         *
         * @param screenMagneticState ScreenMagneticState.
         */
        virtual void OnScreenMagneticStateChanged([[maybe_unused]]bool isMagneticState) {}
    };

    class IBrightnessInfoListener : public virtual RefBase {
    public:
        /**
         * @brief Notify listeners when screen brightness info changed.
         *
         * @param displayId DisplayId.
         * @param info ScreenBrightnessInfo.
         */
        virtual void OnBrightnessInfoChanged([[maybe_unused]]DisplayId displayId, const ScreenBrightnessInfo& info) {}
    };

    class IAvailableAreaListener : public virtual RefBase {
    public:
        /**
         * @brief Notify listeners when available area changed.
         *
         * @param DMRect area.
         */
        virtual void OnAvailableAreaChanged(DMRect area) {}

        /**
         * @brief Notify listeners when available area changed by displayId.
         *
         * @param DMRect area.
         * @param DisplayId displayId.
         */
        virtual void OnAvailableAreaChangedByDisplayId(const DMRect& area, DisplayId displayId) {}
    };

    /**
     * @brief Obtain all displays.
     *
     * @return All displays.
     */
    std::vector<sptr<Display>> GetAllDisplays(int32_t userId = CONCURRENT_USER_ID_DEFAULT);

    /**
     * @brief Obtain the id of the default display.
     *
     * @return Default display id.
     */
    DisplayId GetDefaultDisplayId();

    /**
     * @brief Get the default display object.
     *
     * @return Default display object.
     */
    sptr<Display> GetDefaultDisplay();

    /**
     * @brief Get the default display object by means of sync.
     *
     * @return Default display id.
     */
    sptr<Display> GetDefaultDisplaySync(bool isFromNapi = false, int32_t userId = CONCURRENT_USER_ID_DEFAULT);

    /**
     * @brief Provide ability of setting virtual screen as primary for no-screen device.
     *
     * @param screenId ScreenId used in virtual screen.
     * @return set virtual screen as default screen success or not.
    */
    bool SetVirtualScreenAsDefault(ScreenId screenId);

    /**
     * @brief Get the display object by id.
     *
     * @param displayId Id of the target display.
     * @return Default display object.
     */
    sptr<Display> GetDisplayById(DisplayId displayId);

    /**
     * @brief Get the display object by id.Only for PC.
     *
     * @param displayId Id of the target display.
     * @return Default display object.
     */
    sptr<DisplayInfo> GetVisibleAreaDisplayInfoById(DisplayId displayId);

    /**
     * @brief get available area of the display.(the screen area without dock and statusbar)
     *
     * @param displayId Id of the target display.
     * @param area available area of the screen.
     * @return DMError
     */
    DMError GetExpandAvailableArea(DisplayId displayId, DMRect& area);

    /**
     * @brief Get the display object by corresponding screenId.
     *
     * @param screenId The id of the target screen.
     * @return Display object.
     */
    sptr<Display> GetDisplayByScreen(ScreenId screenId);

    /**
     * @brief Get IDs of all displays.
     *
     * @return All display IDs.
     */
    std::vector<DisplayId> GetAllDisplayIds(int32_t userId = CONCURRENT_USER_ID_DEFAULT);

    /**
     * @brief Get whether the target display contains privacy windows.
     *
     * @param displayId Display id.
     * @param hasPrivateWindow True means display has private window, false means the opposite.
     * @return DM_OK means query privacy success, others means query failed.
     */
    DMError HasPrivateWindow(DisplayId displayId, bool& hasPrivateWindow);

    /**
     * @brief Get screenshot of the target display.
     *
     * @param displayId Display id.
     * @param errorCode error code.
     * @param isUseDma Whether to use DMA, not used by default.
     * @param isCaptureFullOfScreen Whether to take screenshots of all displays on this screen.
     * @return PixelMap object of screenshot.
     */
    std::shared_ptr<Media::PixelMap> GetScreenshot(DisplayId displayId,
        DmErrorCode* errorCode = nullptr, bool isUseDma = false, bool isCaptureFullOfScreen = false);
    
    /**
     * @brief Get SDR and HDR screenshots of the specified display.
     *
     * @param displayId Display id.
     * @param errorCode Error code.
     * @param isUseDma Whether to use DMA, not used by default.
     * @param isCaptureFullOfScreen Whether to take screenshots of all displays on this screen.
     * @return std::vector<std::shared_ptr<Media::PixelMap>> Vector of screenshot pixel maps.
     */
    std::vector<std::shared_ptr<Media::PixelMap>> GetScreenHDRshot(DisplayId displayId,
        DmErrorCode& errorCode, bool isUseDma = false, bool isCaptureFullOfScreen = false);

    /**
     * @brief Get screenshot by user select area.
     *
     * @param rect user select area.
     * @param errorCode error code.
     * @return PixelMap object of screenshot.
     */
    std::shared_ptr<Media::PixelMap> GetSnapshotByPicker(Media::Rect &rect, DmErrorCode* errorCode = nullptr);

    /**
     * @brief Get screenshot of the target display.
     *
     * @param displayId Display id.
     * @param rect Rect of screenshot.
     * @param size Size of screenshot.
     * @param rotation Parameter of rotation.
     * @param errorCode error code.
     * @return PixelMap object of screenshot.
     */
    std::shared_ptr<Media::PixelMap> GetScreenshot(DisplayId displayId, const Media::Rect &rect,
        const Media::Size &size, int rotation, DmErrorCode* errorCode = nullptr);

    /**
     * @brief Get screenshot with option.
     *
     * @param snapShotConfig Parameter of rotation.
     * @param errorCode error code.
     * @return PixelMap object of screenshot.
     */
    std::shared_ptr<Media::PixelMap> GetScreenshotwithConfig(const SnapShotConfig &snapShotConfig,
        DmErrorCode* errorCode = nullptr, bool isUseDma = false);

    /**
     * @brief Begin to wake up screen.
     *
     * @param reason Reason for power state change.
     * @return True means begin success, false means begin failed.
     */
    bool WakeUpBegin(PowerStateChangeReason reason);

    /**
     * @brief Wake up screen end.
     *
     * @return True means end success, false means end failed.
     */
    bool WakeUpEnd();

    /**
     * @brief Begin to suspend the screen.
     *
     * @param reason Reason for power state change.
     * @return True means begin success, false means begin failed.
     */
    bool SuspendBegin(PowerStateChangeReason reason);

    /**
     * @brief Suspend screen end.
     *
     * @return True means suspend screen end success.
     * @return False means suspend screen end failed.
     */
    bool SuspendEnd();

    /**
     * @brief Get id of internal screen.
     *
     * @return Internal screen id.
     */
    ScreenId GetInternalScreenId();

    /**
     * @brief Set the screen power state by screen id.
     *
     * @param screenId Screen id.
     * @param state Screen power state.
     * @param reason Reason for power state change.
     * @return True means set success, false means set failed.
     */
    bool SetScreenPowerById(ScreenId screenId, ScreenPowerState state, PowerStateChangeReason reason);

    /**
     * @brief Set the Display State object
     *
     * @param state State of display.
     * @param callback Callback for display state.
     * @return True means set success, false means set failed.
     */
    bool SetDisplayState(DisplayState state, DisplayStateCallback callback);

    /**
     * @brief Get the state of the target display.
     *
     * @param displayId Display id.
     * @return State of display.
     */
    DisplayState GetDisplayState(DisplayId displayId);

    /**
     * @brief Try to cancel screenoff action before display power off.
     *
     * @return True means cancel screenoff action success.
     * @return False means cancel screenoff action failed.
     */
    bool TryToCancelScreenOff();

    /**
     * @brief Set the brightness level of the target screen.
     *
     * @param screenId Target screen.
     * @param level Brightness level.
     */
    bool SetScreenBrightness(uint64_t screenId, uint32_t level);

    /**
     * @brief Get the brightness level of the target screen.
     *
     * @param screenId Screen id.
     * @return Brightness value of screen.
     */
    uint32_t GetScreenBrightness(uint64_t screenId) const;

    /**
     * @brief Notify when events of certain types occur.
     *
     * @param event Enumerate value of DisplayEvent.
     */
    void NotifyDisplayEvent(DisplayEvent event);

    /**
     * @brief Freeze target displays.
     *
     * @param displayIds Display id needed to freeze.
     * @return True means freeze success, false means freeze failed.
     */
    bool Freeze(std::vector<DisplayId> displayIds);

    /**
     * @brief Unfreeze target displays.
     *
     * @param displayIds Display id needed to unfreeze.
     * @return True means unfreeze success, false means unfreeze failed.
     */
    bool Unfreeze(std::vector<DisplayId> displayIds);

    /**
     * @brief Register a display listener.
     *
     * @param listener IDisplayListener.
     * @return DM_OK means register success, others means register failed.
     */
    DMError RegisterDisplayListener(sptr<IDisplayListener> listener);

    /**
     * @brief Unregister an existed display listener.
     *
     * @param listener IDisplayListener.
     * @return DM_OK means unregister success, others means unregister failed.
     */
    DMError UnregisterDisplayListener(sptr<IDisplayListener> listener);

    /**
     * @brief Register a display attribute listener.
     *
     * @param listener IDisplayAttributeListener.
     * @return DM_OK means register success, others means register failed.
     */
    DMError RegisterDisplayAttributeListener(std::vector<std::string>& attributes,
        sptr<IDisplayAttributeListener> listener);

    /**
     * @brief Unregister an existed display attribute listener.
     *
     * @param listener IDisplayAttributeListener.
     * @return DM_OK means unregister success, others means unregister failed.
     */
    DMError UnRegisterDisplayAttributeListener(sptr<IDisplayAttributeListener> listener);

    /**
     * @brief Register a listener for display power events.
     *
     * @param listener IDisplayPowerEventListener.
     * @return DM_OK means register success, others means register failed.
     */
    DMError RegisterDisplayPowerEventListener(sptr<IDisplayPowerEventListener> listener);

    /**
     * @brief Unregister an existed listener for display power events.
     *
     * @param listener IDisplayPowerEventListener.
     * @return DM_OK means unregister success, others means unregister failed.
     */
    DMError UnregisterDisplayPowerEventListener(sptr<IDisplayPowerEventListener> listener);

    /**
     * @brief Register a listener for screenshot event.
     *
     * @param listener IScreenshotListener.
     * @return DM_OK means register success, others means register failed.
     */
    DMError RegisterScreenshotListener(sptr<IScreenshotListener> listener);

    /**
     * @brief Unregister an existed listener for screenshot event.
     *
     * @param listener IScreenshotListener.
     * @return DM_OK means unregister success, others means unregister failed.
     */
    DMError UnregisterScreenshotListener(sptr<IScreenshotListener> listener);

    /**
     * @brief Register a listener for the event of private window.
     *
     * @param listener IPrivateWindowListener.
     * @return DM_OK means register success, others means register failed.
     */
    DMError RegisterPrivateWindowListener(sptr<IPrivateWindowListener> listener);

    /**
     * @brief Unregister an existed listener for the event of private window.
     *
     * @param listener IPrivateWindowListener.
     * @return DM_OK means unregister success, others means unregister failed.
     */
    DMError UnregisterPrivateWindowListener(sptr<IPrivateWindowListener> listener);

    /**
     * @brief Register a listener for the event of private window.
     *
     * @param listener IPrivateWindowListChangeListener.
     * @return DM_OK means register success, others means register failed.
     */
    DMError RegisterPrivateWindowListChangeListener(sptr<IPrivateWindowListChangeListener> listener);

    /**
     * @brief Unregister an existed listener for the event of private window.
     *
     * @param listener IPrivateWindowListChangeListener.
     * @return DM_OK means unregister success, others means unregister failed.
     */
    DMError UnregisterPrivateWindowListChangeListener(sptr<IPrivateWindowListChangeListener> listener);

    /**
     * @brief Register a listener for the event of screen fold status changed.
     *
     * @param listener IFoldStatusListener.
     * @return DM_OK means register success, others means register failed.
     */
    DMError RegisterFoldStatusListener(sptr<IFoldStatusListener> listener);

    /**
     * @brief Unregister an existed listener for the event of screen fold status changed.
     *
     * @param listener IFoldStatusListener.
     * @return DM_OK means unregister success, others means unregister failed.
     */
    DMError UnregisterFoldStatusListener(sptr<IFoldStatusListener> listener);

    /**
     * @brief Register a listener for the event of screen fold angle changed.
     *
     * @param listener IFoldAngleListener.
     * @return DM_OK means register success, others means register failed.
     */
    DMError RegisterFoldAngleListener(sptr<IFoldAngleListener> listener);

    /**
     * @brief Unregister an existed listener for the event of screen fold angle changed.
     *
     * @param listener IFoldAngleListener.
     * @return DM_OK means unregister success, others means unregister failed.
     */
    DMError UnregisterFoldAngleListener(sptr<IFoldAngleListener> listener);

    /**
     * @brief Register a listener for the event of screen capture status changed.
     *
     * @param listener ICaptureStatusListener.
     * @return DM_OK means register success, others means register failed.
     */
    DMError RegisterCaptureStatusListener(sptr<ICaptureStatusListener> listener);

    /**
     * @brief Unregister an existed listener for the event of screen capture status changed.
     *
     * @param listener ICaptureStatusListener.
     * @return DM_OK means unregister success, others means unregister failed.
     */
    DMError UnregisterCaptureStatusListener(sptr<ICaptureStatusListener> listener);

    /**
     * @brief Register an listener when session changed.
     *
     * @param listener IDisplayUpdateListener.
     * @return DM_OK means register success, others means unregister failed.
     */
    DMError RegisterDisplayUpdateListener(sptr<IDisplayUpdateListener> listener);

    /**
     * @brief Unregister an listener when session changed.
     *
     * @param listener IDisplayUpdateListener.
     * @return DM_OK means unregister success, others means unregister failed.
     */
    DMError UnregisterDisplayUpdateListener(sptr<IDisplayUpdateListener> listener);

    /**
     * @brief Register a listener for the event of display mode changed.
     *
     * @param listener IDisplayModeListener.
     * @return DM_OK means register success, others means register failed.
     */
    DMError RegisterDisplayModeListener(sptr<IDisplayModeListener> listener);

    /**
     * @brief Unregister an existed listener for the event of display mode changed.
     *
     * @param listener IDisplayModeListener.
     * @return DM_OK means unregister success, others means unregister failed.
     */
    DMError UnregisterDisplayModeListener(sptr<IDisplayModeListener> listener);

    /**
     * @brief Register a listener for the event of screen magnetic state changed.
     *
     * @param listener IScreenMagneticStateListener.
     * @return DM_OK means register success, others means register failed.
     */
    DMError RegisterScreenMagneticStateListener(sptr<IScreenMagneticStateListener> listener);

    /**
     * @brief Unregister an existed listener for the event of screen magnetic state changed.
     *
     * @param listener IScreenMagneticStateListener.
     * @return DM_OK means unregister success, others means unregister failed.
     */
    DMError UnregisterScreenMagneticStateListener(sptr<IScreenMagneticStateListener> listener);

    /**
     * @brief Register a listener for the event of brightness info changed.
     *
     * @param listener IBrightnessInfoListener.
     * @return DM_OK means register success, others means register failed.
     */
    DMError RegisterBrightnessInfoListener(sptr<IBrightnessInfoListener> listener);

    /**
     * @brief Unregister an existed listener for the event of brightness info changed.
     *
     * @param listener IBrightnessInfoListener.
     * @return DM_OK means unregister success, others means unregister failed.
     */
    DMError UnregisterBrightnessInfoListener(sptr<IBrightnessInfoListener> listener);

    /**
     * @brief Register a listener for the event of available  area changed.
     *
     * @param listener IAvailableAreaListener.
     * @return DM_OK means unregister success, others means unregister failed.
     */
    DMError RegisterAvailableAreaListener(sptr<IAvailableAreaListener> listener);

    /**
     * @brief Register a listener for the event of available  area changed.
     *
     * @param listener IAvailableAreaListener.
     * @return DM_OK means unregister success, others means unregister failed.
     */
    DMError RegisterAvailableAreaListener(sptr<IAvailableAreaListener> listener, DisplayId displayId);

    /**
     * @brief UnRegister a listener for the event of available  area changed.
     *
     * @param listener IAvailableAreaListener.
     * @return DM_OK means unregister success, others means unregister failed.
     */
    DMError UnregisterAvailableAreaListener(sptr<IAvailableAreaListener> listener);

    /**
     * @brief UnRegister a listener for the event of available  area changed.
     *
     * @param listener IAvailableAreaListener.
     * @return DM_OK means unregister success, others means unregister failed.
     */
    DMError UnregisterAvailableAreaListener(sptr<IAvailableAreaListener> listener, DisplayId displayId);

    /**
     * @brief Add a surface node to the target display.
     *
     * @param displayId Target display.
     * @param surfaceNode SurfaceNode object.
     * @return DM_OK means add success, others means add failed.
     */
    DMError AddSurfaceNodeToDisplay(DisplayId displayId, std::shared_ptr<class RSSurfaceNode>& surfaceNode);

    /**
     * @brief Remove a surface node from the target display.
     *
     * @param displayId Target display.
     * @param surfaceNode SurfaceNode object.
     * @return DM_OK means remove success, others means remove failed.
     */
    DMError RemoveSurfaceNodeFromDisplay(DisplayId displayId, std::shared_ptr<class RSSurfaceNode>& surfaceNode);

    /**
     * @brief Check whether the device is foldable.
     *
     * @return true means the device is foldable.
     */
    bool IsFoldable();

    /**
     * @brief Check whether the device is capture.
     *
     * @return true means the device is capture.
     */
    bool IsCaptured();

    /**
     * @brief Get the current fold status of the foldable device.
     *
     * @return locked fold status if set; otherwise, return the current(actual) fold status.
     */
    FoldStatus GetFoldStatus();

    /**
     * @brief Get the display mode of the foldable device.
     *
     * @return display mode of the foldable device.
     */
    FoldDisplayMode GetFoldDisplayMode();

    /**
     * @brief Get the display mode of the foldable device for external.
     *
     * @return display mode of the foldable device.
     */
    FoldDisplayMode GetFoldDisplayModeForExternal();

    /**
     * @brief Change the display mode of the foldable device.
     *
     * @param mode target display mode to change.
     */
    void SetFoldDisplayMode(const FoldDisplayMode mode);

    /**
     * @brief Change the display mode of the foldable device asynchronously.
     *
     * @param mode target display mode to change.
     */
    void SetFoldDisplayModeAsync(const FoldDisplayMode mode);

    /**
     * @brief Change the display mode of the foldable device from js.
     *
     * @param mode target display mode to change.
     * @param reason display mode change reason.
     * @return DM_OK means set success, others means set failed.
     */
    DMError SetFoldDisplayModeFromJs(const FoldDisplayMode mode, std::string reason = "");

    /**
     * @brief Set display scale.
     *
     * @param screenId screenId used in DisplayManager.
     * @param scaleX screen scale in x axis.
     * @param scaleY screen scale in y axis.
     * @param pivotX screen scale pivot in x axis.
     * @param pivotY screen scale pivot in y axis.
     */
    void SetDisplayScale(ScreenId screenId, float scaleX, float scaleY, float pivotX, float pivotY);

    /**
     * @brief Locked fold status.
     *
     * @param mode locked fold status is locked.
     */
    void SetFoldStatusLocked(bool locked);

    /**
     * @brief Locked fold status from js.
     *
     * @param locked locked fold status is locked.
     * @return DM_OK means set success, others means set failed.
     */
    DMError SetFoldStatusLockedFromJs(bool locked);

    /**
     * @brief Locked fold and set to target fold status
     *
     * @param foldstatus specify fold status to switch to
     * @return DM_OK means set success, others means set failed
     */
    DMError ForceSetFoldStatusAndLock(FoldStatus targetFoldStatus);

    /**
     * @brief Unlock fold status and restore display mode to actual physical fold status
     *
     * @return DM_OK means set success, others means set failed
     */
    DMError RestorePhysicalFoldStatus();

    /**
     * @brief Get the fold crease region in the current display mode.
     *
     * @return fold crease region in the current display mode.
     */
    sptr<FoldCreaseRegion> GetCurrentFoldCreaseRegion();

    /**
     * @brief convert screenId to RsScreenId.
     *
     * @param screenId screenId used in DisplayManager.
     * @param rsScreenId screenId used in RenderService.
     *
     * @return convert success or not.
     */
    bool ConvertScreenIdToRsScreenId(ScreenId screenId, ScreenId& rsScreenId);

    /**
     * @brief get to freeze status with specified pid list
     *
     * @param pidList Indicates the calling pid
     * @param isProxy value is true indicates process status is freeze
     * @param DM_OK means process status update success, others means update failed.
    */
    DMError ProxyForFreeze(std::set<int32_t> pidList, bool isProxy);

    /**
     * @brief reset all process freeze status
     *
     * @param DM_OK means process status update success, others means update failed.
    */
    DMError ResetAllFreezeStatus();

    /**
     * @brief Set virtual screen black list to RS.
     *
     * @param screenId ScreenId used in virtual screen.
     * @param windowIdList The windowId list to shield on cast screen.
     * @param surfaceIdList The surfaceId list to shield on cast screen.
     * @param typeBlackList The surface type list to shield on cast screen.
    */
    void SetVirtualScreenBlackList(ScreenId screenId, std::vector<uint64_t>& windowIdList,
        std::vector<uint64_t> surfaceIdList = {}, std::vector<uint8_t> typeBlackList = {});

    /**
     * @brief Set virtual display mute flag to RS.
     *
     * @param screenId ScreenId used in virtual screen.
     * @param muteFlag The mute flag.
    */
    void SetVirtualDisplayMuteFlag(ScreenId screenId, bool muteFlag);

    /**
     * @brief Determine whether the display is onboard.
     *
     * @param displayId display id.
     * @return displayid whether onboard
    */
    bool IsOnboardDisplay(DisplayId displayId);

    /**
     * @brief When casting the screen, the display not be skipped after the physical screen is turned off.
     *
     * @param screenId ScreenId used in virtual screen.
    */
    void DisablePowerOffRenderControl(ScreenId screenId);

    /**
     * @brief get all display physical resolution
     *
     * @return all physical resolution
     */
    std::vector<DisplayPhysicalResolution> GetAllDisplayPhysicalResolution();

    /**
     * @brief set virtual screen security exemption
     *
     * @param DM_OK means set exemption is success.
    */
    DMError SetVirtualScreenSecurityExemption(ScreenId screenId, uint32_t pid, std::vector<uint64_t>& windowIdList);

    constexpr static int32_t MAX_RESOLUTION_SIZE_SCREENSHOT = 3840; // max resolution, 4K

    /**
     * @brief Add displayId for current ability through Ability Management.
     *
     * @param displayId Identifier of the current display.
     * @param abilityToken Token of the ability.
     */
    void AddDisplayIdFromAms(DisplayId displayId, const wptr<IRemoteObject>& abilityToken);

    /**
     * @brief Removes the display identifier through the Ability Management.
     *
     * @param abilityToken Token of ability.
     */
    void RemoveDisplayIdFromAms(const wptr<IRemoteObject>& abilityToken);

    /**
     * @brief update displayId for current ability through Ability Management.
     *
     * @param displayId Identifier of the current display.
     * @param abilityToken Token of the ability.
     */
    void UpdateDisplayIdFromAms(DisplayId displayId, const wptr<IRemoteObject>& abilityToken);

    /**
     * @brief Get primary display object by means of sync.
     *
     * @return primary display.
     */
    sptr<Display> GetPrimaryDisplaySync();

    /**
     * @brief Get primary display id.
     *
     * @return primary display id.
     */
    DisplayId GetPrimaryDisplayId();

    /**
     * @brief Get screen capture of the target display.
     *
     * @param captureOption screen capture option.
     * @param errorCode error code.
     * @return PixelMap object of screen capture.
     */
    std::shared_ptr<Media::PixelMap> GetScreenCapture(const CaptureOption& captureOption,
        DmErrorCode* errorCode = nullptr);

    /**
     * @brief Get screenshot with capture option.
     *
     * @param captureOption  screen capture option.
     * @param errorCode error code.
     * @return PixelMap object of screenshot.
     */
    std::shared_ptr<Media::PixelMap> GetScreenshotWithOption(const CaptureOption& captureOption,
        DmErrorCode* errorCode = nullptr);

    /**
     * @brief Get screenshot with capture option.
     *
     * @param captureOption  screen capture option.
     * @param rect Rect of screenshot.
     * @param size Size of screenshot.
     * @param rotation Parameter of rotation.
     * @param errorCode error code.
     * @return PixelMap object of screenshot.
     */
    std::shared_ptr<Media::PixelMap> GetScreenshotWithOption(const CaptureOption& captureOption,
        const Media::Rect &rect, const Media::Size &size, int rotation, DmErrorCode* errorCode = nullptr);

    /**
     * @brief Get SDR and HDR screenshot with capture option.
     * @param captureOption  Screen capture option.
     * @param errorCode Error code.
     * @return std::vector<std::shared_ptr<Media::PixelMap>> Vector of screenshot pixel maps.
     */
    std::vector<std::shared_ptr<Media::PixelMap>> GetScreenHDRshotWithOption(
        const CaptureOption& captureOption, DmErrorCode& errorCode);

    /**
     * @brief Get CutoutInfo with rotation
     *
     * @param Rotation rotation.
     * @return CutoutInfo object of default screen.
     */
    sptr<CutoutInfo> GetCutoutInfoWithRotation(Rotation rotation);

    /**
     * @brief Get screenInfo of display area
     *
     * @param DisplayId displayId.
     * @param DMRect displayArea.
     * @param ScreenId screenId.
     * @param DMRect screenArea.
     * @return DM_OK means process get screenArea success.
     */
    DMError GetScreenAreaOfDisplayArea(DisplayId displayId, const DMRect& displayArea,
        ScreenId& screenId, DMRect& screenArea);
    
    /**
     * @brief Get brightnessInfo of display
     *
     * @param DisplayId displayId.
     * @param brightnessInfo ScreenBrightnessInfo.
     * @return DM_OK means process get screenArea success.
     */
    DMError GetBrightnessInfo(DisplayId displayId, ScreenBrightnessInfo& brightnessInfo);

    /**
     * @brief Get primary display system dpi.
     *
     * @return primary display system dpi.
     */
    float GetPrimaryDisplaySystemDpi() const;

    /**
     * @brief Convert a relative position to a global position
     *
     * @param relativePosition Position relative to current display
     * @param position Global position converted from  relativePosition
     * @return DM_OK means process convert position success, others means convert failed.
     */
    DMError ConvertRelativeCoordinateToGlobal(const RelativePosition& relativePosition, Position& position);
    
    /**
     * @brief Convert a global position to a relative position
     *
     * @param globalPosition Global position
     * @param relativePosition Relative position converted from globalPosition
     * @return DM_OK means process convert position success, others means convert failed.
     */
    DMError ConvertGlobalCoordinateToRelative(const Position& globalPosition, RelativePosition& relativePosition);

    /**
     * @brief Convert a global position to a relative position based on specifed ID
     *
     * @param globalPosition Global position
     * @param displayId DisplayId of current display
     * @param relativePosition Relative position converted from globalPosition
     * @return DM_OK means process convert position success, others means convert failed.
     */
    DMError ConvertGlobalCoordinateToRelativeWithDisplayId(const Position& globalPosition, DisplayId displayId,
        RelativePosition& relativePosition);
    
    /**
     * @brief Get supportsInput of display
     *
     * @param DisplayId displayId.
     * @param bool supportsInput.
     * @return DM_OK means the get operation succeeds.
     */
    DMError GetSupportsInput(DisplayId displayId, bool& supportsInput);

    /**
     * @brief Set supportsInput of display
     *
     * @param DisplayId displayId.
     * @param bool supportsInput.
     * @return DM_OK means the set operation succeeds.
     */
    DMError SetSupportsInput(DisplayId displayId, bool supportsInput);

    /**
     * @brief Get bundleName of display
     *
     * @param displayId
     * @param bundleName
     * @return DMError
     */
    DMError GetBundleName(DisplayId displayId, std::string& bundleName);

    /**
     * @brief Unregister display attribute not listening .
     *
     * @param attributesNotListened Attributes which not listening.
     * @return DM_OK means unregister success, others means unregister failed.
     */
    DMError UnRegisterDisplayAttribute(const std::vector<std::string>& attributesNotListened);

private:
    DisplayManager();
    ~DisplayManager();
    void OnRemoteDied();

    void ShowDisplayIdList(bool isShowLog = false);
    std::mutex displayOperateMutex_;
    DisplayId GetCallingAbilityDisplayId();
    std::vector<std::pair<wptr<IRemoteObject>, DisplayId>> displayIdList_ {};
    bool CheckUseGpuScreenshotWithOption(const Media::Rect &rect, const Media::Size &size);
    std::shared_ptr<Media::PixelMap> GetScreenshotWithOptionUseGpu(const CaptureOption& captureOption,
        const Media::Rect &rect, const Media::Size &size, int rotation, DmErrorCode* errorCode = nullptr);
    class Impl;
    std::recursive_mutex mutex_;
    sptr<Impl> pImpl_;
};
} // namespace OHOS::Rosen

#endif // FOUNDATION_DM_DISPLAY_MANAGER_H
