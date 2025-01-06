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

    class IAvailableAreaListener : public virtual RefBase {
    public:
        /**
         * @brief Notify listeners when available area changed.
         *
         * @param DMRect area.
         */
        virtual void OnAvailableAreaChanged(DMRect area) {}
    };

    /**
     * @brief Obtain all displays.
     *
     * @return All displays.
     */
    std::vector<sptr<Display>> GetAllDisplays();

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
    sptr<Display> GetDefaultDisplaySync(bool isFromNapi = false);

    /**
     * @brief Get the display object by id.
     *
     * @param displayId Id of the target display.
     * @return Default display object.
     */
    sptr<Display> GetDisplayById(DisplayId displayId);

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
    std::vector<DisplayId> GetAllDisplayIds();

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
     * @return PixelMap object of screenshot.
     */
    std::shared_ptr<Media::PixelMap> GetScreenshot(DisplayId displayId,
        DmErrorCode* errorCode = nullptr, bool isUseDma = false);

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
     * @brief Register a listener for the event of available  area changed.
     *
     * @param listener IAvailableAreaListener.
     * @return DM_OK means unregister success, others means unregister failed.
     */
    DMError RegisterAvailableAreaListener(sptr<IAvailableAreaListener> listener);

    /**
     * @brief UnRegister a listener for the event of available  area changed.
     *
     * @param listener IAvailableAreaListener.
     * @return DM_OK means unregister success, others means unregister failed.
     */
    DMError UnregisterAvailableAreaListener(sptr<IAvailableAreaListener> listener);

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
     * @return fold status of device.
     */
    FoldStatus GetFoldStatus();

    /**
     * @brief Get the display mode of the foldable device.
     *
     * @return display mode of the foldable device.
     */
    FoldDisplayMode GetFoldDisplayMode();

    /**
     * @brief Change the display mode of the foldable device.
     *
     * @param mode target display mode to change.
     */
    void SetFoldDisplayMode(const FoldDisplayMode mode);

    /**
     * @brief Change the display mode of the foldable device from js.
     *
     * @param mode target display mode to change.
     * @return DM_OK means set success, others means set failed.
     */
    DMError SetFoldDisplayModeFromJs(const FoldDisplayMode mode);

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
    */
    void SetVirtualScreenBlackList(ScreenId screenId, std::vector<uint64_t>& windowIdList);

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
     * @brief Get primary display object by means of sync.
     *
     * @return primary display.
     */
    sptr<Display> GetPrimaryDisplaySync();

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
private:
    DisplayManager();
    ~DisplayManager();
    void OnRemoteDied();

    void ShowDisplayIdList(bool isShowLog = false);
    std::mutex displayOperateMutex_;
    DisplayId GetCallingAbilityDisplayId();
    std::vector<std::pair<wptr<IRemoteObject>, DisplayId>> displayIdList_ {};

    class Impl;
    std::recursive_mutex mutex_;
    sptr<Impl> pImpl_;
};
} // namespace OHOS::Rosen

#endif // FOUNDATION_DM_DISPLAY_MANAGER_H