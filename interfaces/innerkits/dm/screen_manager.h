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

#ifndef FOUNDATION_DM_SCREEN_MANAGER_H
#define FOUNDATION_DM_SCREEN_MANAGER_H

#include <refbase.h>
#include "screen.h"
#include "dm_common.h"
#include "screen_group.h"
#include "wm_single_instance.h"
#include "wm_single_instance.h"
#include <pixel_map.h>

namespace OHOS::Rosen {
class ScreenManager : public RefBase {
WM_DECLARE_SINGLE_INSTANCE_BASE(ScreenManager);
friend class DMSDeathRecipient;
public:
    class IScreenListener : public virtual RefBase {
    public:
        /**
         * @brief Notify when a new screen is connected.
         */
        virtual void OnConnect(ScreenId) = 0;

        /**
         * @brief Notify when a screen is disconnected.
         */
        virtual void OnDisconnect(ScreenId) = 0;

        /**
         * @brief Notify when state of the screen is changed.
         */
        virtual void OnChange(ScreenId) = 0;
    };

    class IScreenGroupListener : public virtual RefBase {
    public:
        /**
         * @brief Notify when state of the screenGroup is changed.
         */
        virtual void OnChange(const std::vector<ScreenId>&, ScreenGroupChangeEvent) = 0;
    };

    class IRecordDisplayListener : public virtual RefBase {
    public:
        /**
         * @brief Notify when recording display changed.
         */
        virtual void OnChange(const std::vector<DisplayId>& displayIds) = 0;
    };

    class IVirtualScreenGroupListener : public virtual RefBase {
    public:
        struct ChangeInfo {
            ScreenGroupChangeEvent event;
            std::string trigger;
            std::vector<ScreenId> ids;
        };
        /**
         * @brief Notify when an event related to screen mirror occurs.
         *
         * @param info Change info of screen mirror.
         */
        virtual void OnMirrorChange([[maybe_unused]]const ChangeInfo& info) {}
    };

    /**
     * @brief Get the screen object by screen id.
     *
     * @param screenId Screen id.
     * @return Screen object.
     */
    sptr<Screen> GetScreenById(ScreenId screenId);

    /**
     * @brief Get the screen group object by groupId.
     *
     * @param groupId Screen group id.
     * @return ScreenGroup object.
     */
    sptr<ScreenGroup> GetScreenGroup(ScreenId groupId);

    /**
     * @brief Get all screens object.
     *
     * @param screens All screen objects.
     * @return DM_OK means get success, others means get failed.
     */
    DMError GetAllScreens(std::vector<sptr<Screen>>& screens);

    /**
     * @brief Make screens as expand-screen.
     *
     * @param options Option of expand.
     * @param screenGroupId Screen group id.
     * @return DM_OK means make expand success, others means make expand failed.
     */
    DMError MakeExpand(const std::vector<ExpandOption>& options, ScreenId& screenGroupId);

    /**
     * @brief Make screens as mirror-screen
     *
     * @param mainScreenId Main screen id.
     * @param mirrorScreenId Mirror screen ids.
     * @param screenGroupId Screen group id.
     * @return DM_OK means make mirror success, others means make mirror failed.
     */
    DMError MakeMirror(ScreenId mainScreenId, std::vector<ScreenId> mirrorScreenId, ScreenId& screenGroupId);

     /**
     * @brief Make screens as mirror-screen
     *
     * @param mainScreenId Main screen id.
     * @param mirrorScreenId Mirror screen ids.
     * @param screenGroupId Screen group id.
     * @return DM_OK means make mirror success, others means make mirror failed.
     */
    DMError MakeMirrorForRecord(const std::vector<ScreenId>& mainScreenIds,
        std::vector<ScreenId>& mirrorScreenIds, ScreenId& screenGroupId);

    /**
     * @brief Make screen as mirror-screen for region of main screen.
     *
     * @param mainScreenId Main screen id.
     * @param mirrorScreenId Mirror screen ids.
     * @param mainScreenRegion Region of main screen.
     * @param screenGroupId Screen group id.
     * @return DM_OK means make mirror success, others means make mirror failed.
     */
    DMError MakeMirror(ScreenId mainScreenId, std::vector<ScreenId> mirrorScreenId, DMRect mainScreenRegion,
        ScreenId& screenGroupId);

    /**
     * @brief Make screen as mirror-screen with rotation.
     *
     * @param mainScreenId Main screen id.
     * @param mirrorScreenId Mirror screen ids.
     * @param screenGroupId Screen group id.
     * @param rotation Mirror screen rotation.
     * @return DM_OK means make mirror success, others means make mirror failed.
     */
    DMError MakeMirror(ScreenId mainScreenId, std::vector<ScreenId> mirrorScreenId,
        ScreenId& screenGroupId, Rotation rotation);

    /**
     * @brief Make screens as mirror-screen
     *
     * @param mainScreenId Main screen id.
     * @param secondaryScreenId secondary screen id.
     * @param screenMode Screen Combination Mode.
     * @return DM_OK means make mirror success, others means make mirror failed.
     */
    DMError SetMultiScreenMode(ScreenId mainScreenId, ScreenId secondaryScreenId,
        MultiScreenMode screenMode);

    /**
     * @brief Set Screen Relative Position
     *
     * @param mainScreenOptions Main screen id and position.
     * @param secondScreenOption secondary screen id and position.
     * @return DM_OK means make mirror success, others means make mirror failed.
     */
    DMError SetMultiScreenRelativePosition(MultiScreenPositionOptions mainScreenOptions,
        MultiScreenPositionOptions secondScreenOption);

    /**
     * @brief Make screens as unique-screen.
     *
     * @param screenIds Unique screen ids.
     * @param rotationOptions Parameter controlling whether screen rotation is locked when creating unique screen
     * @return DM_OK means make unique screen success, others means make unique failed.
     */
    DMError MakeUniqueScreen(const std::vector<ScreenId>& screenIds,
        const UniqueScreenRotationOptions& rotationOptions);

    /**
     * @brief Make screens as unique-screen.
     *
     * @param screenIds Unique screen ids.
     * @return DM_OK means make unique screen success, others means make unique failed.
     */
    DMError MakeUniqueScreen(const std::vector<ScreenId>& screenIds);

    /**
     * @brief Make screens as unique-screen.
     *
     * @param screenIds Unique screen ids.
     * @param displayIds Unique display ids.
     * @param rotationOptions Parameter controlling whether screen rotation is locked when creating unique screen
     * @return DM_OK means make unique screen success, others means make unique failed.
     */
    DMError MakeUniqueScreen(const std::vector<ScreenId>& screenIds, std::vector<DisplayId>& displayIds,
        const UniqueScreenRotationOptions& rotationOptions);

    /**
     * @brief Make screens as unique-screen.
     *
     * @param screenIds Unique screen ids.
     * @param displayIds Unique display ids.
     * @return DM_OK means make unique screen success, others means make unique failed.
     */
    DMError MakeUniqueScreen(const std::vector<ScreenId>& screenIds, std::vector<DisplayId>& displayIds);

    /**
     * @brief Stop expand screens.
     *
     * @param expandScreenIds Expand screen ids.
     * @return DM_OK means stop expand success, others means stop expand failed.
     */
    DMError StopExpand(const std::vector<ScreenId>& expandScreenIds);

    /**
     * @brief Stop mirror screens.
     *
     * @param mirrorScreenIds Mirror screen ids.
     * @return DM_OK means stop mirror success, others means stop mirror failed.
     */
    DMError StopMirror(const std::vector<ScreenId>& mirrorScreenIds);

    /**
     * @brief Disable/enable global mirror screen.
     *
     * @param disableOrNot disable mirror screen or not.
     * @return DM_OK means disable or enable mirror success, others means failed.
     */
    DMError DisableMirror(bool disableOrNot);

    /**
     * @brief Remove virtual screen from group.
     *
     * @param screens Screen ids.
     * @return DM_OK means remove success, others means remove failed.
     */
    DMError RemoveVirtualScreenFromGroup(std::vector<ScreenId> screens);

    /**
     * @brief Add virtual screen whitelist.
     *
     * @param screenId Screen id.
     * @param missionIds mission ids.
     * @return DM_OK means add success, others means add failed.
     */
    DMError AddVirtualScreenWhiteList(ScreenId screenId, const std::vector<uint64_t>& missionIds);

    /**
     * @brief Remove virtual screen whitelist.
     *
     * @param screenId Screen id.
     * @param missionIds mission ids.
     * @return DM_OK means remove success, others means remove failed.
     */
    DMError RemoveVirtualScreenWhiteList(ScreenId screenId, const std::vector<uint64_t>& missionIds);

    /**
     * @brief Create virtual screen.
     *
     * @param option Indicates the options of the virtual screen.
     * @return Screen id.
     */
    ScreenId CreateVirtualScreen(VirtualScreenOption option);

    /**
     * @brief Destroy virtual screen.
     *
     * @param screenId Indicates the screen id of the virtual screen.
     * @return DM_OK means destroy success, others means destroy failed.
     */
    DMError DestroyVirtualScreen(ScreenId screenId);

    /**
     * @brief Set surface for the virtual screen.
     *
     * @param screenId Screen id.
     * @param surface Surface object.
     * @return DM_OK means set success, others means set failed.
     */
    DMError SetVirtualScreenSurface(ScreenId screenId, sptr<Surface> surface);

    /**
     * @brief Add black window list in virtual screen
     *
     * @param persistentIds Black list window ids.
     * @return DM_OK means set success, others means set failed.
     */
    DMError AddVirtualScreenBlockList(const std::vector<int32_t>& peristentIds);

    /**
     * @brief Remove black window list in virtual screen
     *
     * @param persistentIds Black list window ids.
     * @return DM_OK means set success, others means set failed.
     */
    DMError RemoveVirtualScreenBlockList(const std::vector<int32_t>& peristentIds);

    /**
     * @brief Set privacy image.
     *
     * @param screenId Screen id.
     * @param privacyMaskImg PixelMap object.
     * @return DM_OK means set success, others means set failed.
     */
    DMError SetScreenPrivacyMaskImage(ScreenId screenId, const std::shared_ptr<Media::PixelMap>& privacyMaskImg);

    /**
     * @brief Set canvas auto rotate
     *
     * @param screenId Screen id.
     * @param canvasRotation auto rotate
     * @return DM_OK means set success, others means set failed.
     */
    DMError SetVirtualMirrorScreenCanvasRotation(ScreenId screenId, bool canvasRotation);

    /**
     * @brief Resize virtual screen
     *
     * @param screenId the id of virtual screen to be resized.
     * @param width the new width.
     * @param height the new height.
     * @return DM_OK means set success, others means set failed.
     */
    DMError ResizeVirtualScreen(ScreenId screenId, uint32_t width, uint32_t height);

    /**
     * @brief Set virtual screen scale mode
     *
     * @param screenId Screen id.
     * @param scaleMode scale mode
     * @return DM_OK means set success, others means set failed.
     */
    DMError SetVirtualMirrorScreenScaleMode(ScreenId screenId, ScreenScaleMode scaleMode);

    /**
     * @brief Set the screen power state on the specified screen.
     *
     * @param screenId Screen id.
     * @param state Screen power state.
     * @param reason Reason for power state change.
     * @return True means set success, false means set failed.
     */
    bool SetSpecifiedScreenPower(ScreenId screenId, ScreenPowerState state, PowerStateChangeReason reason);

    /**
     * @brief Set the screen power states for all screens.
     *
     * @param state Screen power state.
     * @param reason Reason for power state change.
     * @return True means set success, false means set failed.
     */
    bool SetScreenPowerForAll(ScreenPowerState state, PowerStateChangeReason reason);

    /**
     * @brief Get screen power state.
     *
     * @param screenId Screen id.
     * @return Power state of screen.
     */
    ScreenPowerState GetScreenPower(ScreenId screenId);

    /**
     * @brief Get screen power state.
     *
     * @return Power state of screen.
     */
    ScreenPowerState GetScreenPower();

    /**
     * @brief Set screen rotation lock status.
     *
     * @param isLocked True means forbid to rotate screen, false means the opposite.
     * @return DM_OK means set success, others means set failed.
     */
    DMError SetScreenRotationLocked(bool isLocked);

    /**
     * @brief Set screen rotation lock status from js.
     *
     * @param isLocked True means forbid to rotate screen, false means the opposite.
     * @return DM_OK means set success, others means set failed.
     */
    DMError SetScreenRotationLockedFromJs(bool isLocked);

    /**
     * @brief Get screen rotation lock status.
     *
     * @param isLocked Query the rotation lock status.
     * @return DM_OK means query success, others means query failed.
     */
    DMError IsScreenRotationLocked(bool& isLocked);

    /**
     * @brief Register screen listener.
     *
     * @param listener IScreenListener.
     * @return DM_OK means register success, others means register failed.
     */
    DMError RegisterScreenListener(sptr<IScreenListener> listener);

    /**
     * @brief Unregister screen listener.
     *
     * @param listener IScreenListener.
     * @return DM_OK means unregister success, others means unregister failed.
     */
    DMError UnregisterScreenListener(sptr<IScreenListener> listener);

    /**
     * @brief Register screen group listener.
     *
     * @param listener IScreenGroupListener.
     * @return DM_OK means register success, others means register failed.
     */
    DMError RegisterScreenGroupListener(sptr<IScreenGroupListener> listener);

    /**
     * @brief Unregister screen group listener.
     *
     * @param listener IScreenGroupListener.
     * @return DM_OK means unregister success, others means unregister failed.
     */
    DMError UnregisterScreenGroupListener(sptr<IScreenGroupListener> listener);

    /**
     * @brief Register recording display change listener.
     *
     * @param listener IRecordDisplayListener.
     * @return DM_OK means register success, others means register failed.
     */
    DMError RegisterRecordDisplayListener(sptr<IRecordDisplayListener> listener);

    /**
     * @brief Unregister recording display change listener.
     *
     * @param listener IRecordDisplayListener.
     * @return DM_OK means unregister success, others means unregister failed.
     */
    DMError UnRegisterRecordDisplayListener(sptr<IRecordDisplayListener> listener);

    /**
     * @brief Register virtual screen group listener.
     *
     * @param listener IVirtualScreenGroupListener.
     * @return DM_OK means register success, others means register failed.
     */
    DMError RegisterVirtualScreenGroupListener(sptr<IVirtualScreenGroupListener> listener);

    /**
     * @brief Unregister virtual screen group listener.
     *
     * @param listener IVirtualScreenGroupListener.
     * @return DM_OK means unregister success, others means unregister failed.
     */
    DMError UnregisterVirtualScreenGroupListener(sptr<IVirtualScreenGroupListener> listener);

    /**
     * @brief Get virtual screen flag.
     *
     * @param screenId virtual screen id.
     * @return virtual screen flag
     */
    VirtualScreenFlag GetVirtualScreenFlag(ScreenId screenId);

    /**
     * @brief Set virtual screen flag.
     *
     * @param screenId virtual screen id.
     * @param screenFlag virtual screen flag.
     * @return DM_OK means set success, others means failed.
     */
    DMError SetVirtualScreenFlag(ScreenId screenId, VirtualScreenFlag screenFlag);

    /**
     * @brief Set virtual screen refresh rate with a refresh interval relative to the main screen.
     *
     * @param screenId virtual screen id.
     * @param refreshInterval refresh interval: range [1, main screen max refresh rate]. 2 indicates that
     *     the virtual screen refreshes 1 frame when the main screen refreshes 2 frames, 5 indicates that
     *     the virtual screen refreshes 1 frame when the main screen refreshes 5 frames, and so on.
     * @return DM_OK means set success, others means failed.
     */
    DMError SetVirtualScreenRefreshRate(ScreenId screenId, uint32_t refreshInterval);

    /**
     * @brief Set virtual screen max refresh rate .
     *
     * @param screenId virtual screen id.
     * @param refreshRate max refresh rate to be set.
     * @param actualRefreshRate the actual refresh rate that is finally set.
     * @return DM_OK means set success, others means failed.
     */
    DMError SetVirtualScreenMaxRefreshRate(ScreenId id, uint32_t refreshRate,
        uint32_t& actualRefreshRate);

    /**
     * @brief when casting the screen, Virtual screen playback and pause function.
     *
     * @param screenId screenId used in virtual screen.
     * @param screenStatus VIRTUAL_SCREEN_PLAY or VIRTUAL_SCREEN_PAUSE.
     * @return True means set success, false means set failed.
     */
    bool SetVirtualScreenStatus(ScreenId screenId, VirtualScreenStatus screenStatus);

    /**
     * @brief Set screen sharing protect
     *
     * @param screenIds screen IDs for set the screen sharing protect.
     * @param isEnable wether to enable the screen sharing protect.
     * @return DM_OK means set success, others means failed.
     */
    DMError SetScreenSkipProtectedWindow(const std::vector<ScreenId>& screenIds, bool isEnable);

    /**
     * @brief Set fold status expand and locked, only for super fold display device.
     *
     * @param locked wether to set fold status expand and locked.
     */
    void SetFoldStatusExpandAndLocked(bool locked);

    /**
     * @brief Set that if resize buffer and keep content horizontal while rotating the virtual display.
     *
     * @param screenId virtual screen id.
     * @param enable True means enable, false means disable.
     * @return DM_OK means set success, others means failed.
     */
    DMError SetVirtualScreenAutoRotation(ScreenId screenId, bool enable);

    /**
     * @brief Enable or disable the privacy window of a specified tag on the virtual screen.
     *
     * @param screenId virtual screen id.
     * @param privacyWindowTag tag of the privacy window.
     * @param enable True means enable, false means disable.
     * @return DM_OK means set success, others means failed.
     */
    DMError SetScreenPrivacyWindowTagSwitch(ScreenId screenId, const std::vector<std::string>& privacyWindowTag,
        bool enable);

private:
    ScreenManager();
    ~ScreenManager();
    void OnRemoteDied();

    class Impl;
    sptr<Impl> pImpl_;
};
} // namespace OHOS::Rosen

#endif // FOUNDATION_DM_SCREEN_MANAGER_H
