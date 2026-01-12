/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at,
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software,
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FOUNDATION_DM_DISPLAY_MANAGER_LITE_H
#define FOUNDATION_DM_DISPLAY_MANAGER_LITE_H

#include <vector>
#include <mutex>

#include "display_lite.h"
#include "dm_common.h"
#include "wm_single_instance.h"
#include "display_change_info.h"

namespace OHOS::Rosen {
class DisplayManagerLite {
WM_DECLARE_SINGLE_INSTANCE_BASE(DisplayManagerLite);
friend class DMSDeathRecipientLite;
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

    class IFoldStatusListener : public virtual RefBase {
    public:
        /**
         * @brief Notify listeners when screen fold status changed.
         *
         * @param foldStatus Screen foldStatus.
         */
        virtual void OnFoldStatusChanged([[maybe_unused]]FoldStatus foldStatus) {}
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
     * @brief Get the default display object.
     *
     * @return Default display object.
     */
    sptr<DisplayLite> GetDefaultDisplay(int32_t userId = CONCURRENT_USER_ID_DEFAULT);

    /**
     * @brief Check whether the device is foldable.
     *
     * @return true means the device is foldable.
     */
    bool IsFoldable();

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
     * @brief Get the display mode of the foldable device for external.
     *
     * @return display mode of the foldable device.
     */
    FoldDisplayMode GetFoldDisplayModeForExternal();

    /**
     * @brief Change the display mode of the foldable device.
     *
     * @param mode target display mode to change.
     * 使用场景：目前仅提供给维修场景使用，折叠屏手动色彩校准
     */
    void SetFoldDisplayMode(const FoldDisplayMode mode);

    /**
     * @brief Change the display mode of the foldable device asynchronously.
     *
     * @param mode target display mode to change.
     */
    void SetFoldDisplayModeAsync(const FoldDisplayMode mode);

    /**
     * @brief Get the display object by id.
     *
     * @param displayId Id of the target display.
     * @return Default display object.
     */
    sptr<DisplayLite> GetDisplayById(DisplayId displayId);

    /*
     * used by powermgr
     */
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
     * @brief Obtain the id of the default display.
     *
     * @return Default display id.
     */
    DisplayId GetDefaultDisplayId(int32_t userId = CONCURRENT_USER_ID_DEFAULT);

    /**
     * @brief Get IDs of all displays.
     *
     * @return All display IDs.
     */
    std::vector<DisplayId> GetAllDisplayIds(int32_t userId = CONCURRENT_USER_ID_DEFAULT);

    /**
     * @brief Get virtual screen flag.
     *
     * @param screenId virtual screen id.
     * @return virtual screen flag
     */
    VirtualScreenFlag GetVirtualScreenFlag(ScreenId screenId);

    /**
     * @brief Determine whether the display is onboard.
     *
     * @param displayId display id.
     * @return DMError represent operation result, isOnboardDisplay represent whether displayid is onboard
    */
    DMError IsOnboardDisplay(DisplayId displayId, bool& isOnboardDisplay);

    /**
     * @brief Set System Keyboard status.
     *
     * @param isTpKeyboardOn whether system keyboard with touchpad is shown, default is false.
     * @return DM_OK means set system keyboard status success.
     */
    DMError SetSystemKeyboardStatus(bool isTpKeyboardOn = false);
private:
    DisplayManagerLite();
    ~DisplayManagerLite();
    void OnRemoteDied();

    class Impl;
    std::recursive_mutex mutex_;
    bool destroyed_ = false;
    sptr<Impl> pImpl_;
};
} // namespace OHOS::Rosen

#endif // FOUNDATION_DM_DISPLAY_MANAGER_LITE_H