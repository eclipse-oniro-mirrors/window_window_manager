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

#ifndef OHOS_ROSEN_DISPLAY_MANAGER_IPC_SERVICE_H
#define OHOS_ROSEN_DISPLAY_MANAGER_IPC_SERVICE_H

#include <errors.h>
#include <system_ability.h>
#include <system_ability_definition.h>
#include "display_manager_service.h"
#include "display_manager_stub.h"

namespace OHOS::Rosen {
class DisplayManagerIpcService : public SystemAbility, public DisplayManagerStub {
DECLARE_SYSTEM_ABILITY(DisplayManagerIpcService);
WM_DECLARE_SINGLE_INSTANCE_BASE(DisplayManagerIpcService);

protected:
    void OnStart() override;
    void OnStop() override;

public:
    int Dump(int fd, const std::vector<std::u16string>& args) override;

    ErrCode GetDefaultDisplayInfo(
        sptr<DisplayInfo>& displayInfo) override;

    ErrCode GetDisplayInfoById(
        uint64_t displayId,
        sptr<DisplayInfo>& displayInfo) override;

    ErrCode GetVisibleAreaDisplayInfoById(
        uint64_t displayId,
        sptr<DisplayInfo>& displayInfo) override;

    ErrCode GetDisplayInfoByScreen(
        uint64_t screenId,
        sptr<DisplayInfo>& displayInfo) override;

    ErrCode CreateVirtualScreen(
        const DmVirtualScreenOption& virtualOption,
        const sptr<IRemoteObject>& displayManagerAgent,
        uint64_t& screenId) override;

    ErrCode CreateVirtualScreen(
        const DmVirtualScreenOption& virtualOption,
        const sptr<IRemoteObject>& displayManagerAgent,
        uint64_t& screenId,
        const sptr<IBufferProducer>& surface) override;

    ErrCode DestroyVirtualScreen(
        uint64_t screenId,
        int32_t& dmError) override;

    ErrCode SetVirtualScreenSurface(
        uint64_t screenId,
        const sptr<IBufferProducer>& surface,
        int32_t& dmError) override;

    ErrCode SetOrientation(
        uint64_t screenId,
        uint32_t orientation,
        int32_t& dmError) override;

    ErrCode GetDisplaySnapshot(
        uint64_t displayId,
        int32_t& errorCode,
        bool isUseDma,
        bool isCaptureFullOfScreen,
        std::shared_ptr<PixelMap>& pixelMap) override;

    ErrCode GetScreenSupportedColorGamuts(
        uint64_t screenId,
        std::vector<uint32_t>& colorGamuts,
        int32_t& dmError) override;

    ErrCode GetScreenColorGamut(
        uint64_t screenId,
        uint32_t& colorGamut,
        int32_t& dmError) override;

    ErrCode SetScreenColorGamut(
        uint64_t screenId,
        int32_t colorGamutIdx,
        int32_t& dmError) override;

    ErrCode GetScreenGamutMap(
        uint64_t screenId,
        uint32_t& gamutMap,
        int32_t& dmError) override;

    ErrCode SetScreenGamutMap(
        uint64_t screenId,
        uint32_t gamutMap,
        int32_t& dmError) override;

    ErrCode SetScreenColorTransform(
        uint64_t screenId) override;

    ErrCode RegisterDisplayManagerAgent(
        const sptr<IDisplayManagerAgent>& displayManagerAgent,
        uint32_t type,
        int32_t& dmError) override;

    ErrCode UnregisterDisplayManagerAgent(
        const sptr<IDisplayManagerAgent>& displayManagerAgent,
        uint32_t type,
        int32_t& dmError) override;

    ErrCode WakeUpBegin(
        uint32_t reason,
        bool& isSucc) override;

    ErrCode WakeUpEnd(
        bool& isSucc) override;

    ErrCode SuspendBegin(
        uint32_t reason,
        bool& isSucc) override;

    ErrCode SuspendEnd(
        bool& isSucc) override;

    ErrCode SetScreenPowerForAll(
        uint32_t screenPowerState,
        uint32_t reason,
        bool& isSucc) override;

    ErrCode SetSpecifiedScreenPower(
        uint64_t screenId,
        uint32_t screenPowerState,
        uint32_t reason,
        bool& isSucc) override;

    ErrCode GetScreenPower(
        uint64_t dmsScreenId,
        uint32_t& screenPowerState) override;

    ErrCode SetDisplayState(
        uint32_t displayState,
        bool& isSucc) override;

    ErrCode GetDisplayState(
        uint64_t displayId,
        uint32_t& displayState) override;

    ErrCode TryToCancelScreenOff(
        bool& isSucc) override;

    ErrCode GetAllDisplayIds(
    std::vector<uint64_t>& displayIds) override;

    ErrCode GetCutoutInfo(
        uint64_t displayId,
        sptr<CutoutInfo>& cutoutInfo) override;

    ErrCode AddSurfaceNodeToDisplay(
        uint64_t displayId,
        const std::shared_ptr<DmRsSurfaceNode>& dmRsSurfaceNode,
        int32_t& dmError) override;

    ErrCode RemoveSurfaceNodeFromDisplay(
        uint64_t displayId,
        const std::shared_ptr<DmRsSurfaceNode>& dmRsSurfaceNode,
        int32_t& dmError) override;

    ErrCode HasPrivateWindow(
        uint64_t displayId,
        bool& hasPrivateWindow,
        int32_t& dmError) override;

    ErrCode NotifyDisplayEvent(
        uint32_t displayEvent) override;

    ErrCode SetFreeze(
        const std::vector<uint64_t>& displayIds,
        bool isFreeze,
        bool& isSucc) override;

    ErrCode MakeMirror(
        uint64_t mainScreenId,
        const std::vector<uint64_t>& mirrorScreenId,
        uint64_t& screenGroupId,
        int32_t& dmError) override;

    ErrCode StopMirror(
        const std::vector<uint64_t>& mirrorScreenIds,
        int32_t& dmError) override;

    ErrCode GetScreenInfoById(
        uint64_t screenId,
        sptr<ScreenInfo>& screenInfo) override;

    ErrCode GetScreenGroupInfoById(
        uint64_t screenId,
        sptr<ScreenGroupInfo>& screenGroupInfo) override;

    ErrCode GetAllScreenInfos(
        std::vector<sptr<ScreenInfo>>& screenInfos,
        int32_t& dmError) override;

    ErrCode MakeExpand(
        const std::vector<uint64_t>& screenId,
        const std::vector<Point>& startPoint,
        uint64_t& screenGroupId,
        int32_t& dmError) override;

    ErrCode StopExpand(
        const std::vector<uint64_t>& expandScreenIds,
        int32_t& dmError) override;

    ErrCode RemoveVirtualScreenFromGroup(
        const std::vector<uint64_t>& screens) override;

    ErrCode SetScreenActiveMode(
        uint64_t screenId,
        uint32_t modeId,
        int32_t& dmError) override;

    ErrCode SetVirtualPixelRatio(
        uint64_t screenId,
        float virtualPixelRatio,
        int32_t& dmError) override;

    ErrCode SetResolution(
        uint64_t screenId,
        uint32_t width,
        uint32_t height,
        float virtualPixelRatio,
        int32_t& dmError) override;

    ErrCode GetDensityInCurResolution(
        uint64_t screenId,
        float& virtualPixelRatio,
        int32_t& dmError) override;

    ErrCode IsScreenRotationLocked(
        bool& isLocked,
        int32_t& dmError) override;

    ErrCode SetScreenRotationLocked(
        bool isLocked,
        int32_t& dmError) override;

    ErrCode SetScreenRotationLockedFromJs(
        bool isLocked,
        int32_t& dmError) override;

    ErrCode GetAllDisplayPhysicalResolution(
        std::vector<DisplayPhysicalResolution>& displayPhysicalResolutions) override;

    ErrCode SetScreenBrightness(
        const DmsScreenBrightnessData& brightnessData,
        bool& isSucc) override;

    ErrCode GetScreenBrightness(
        uint64_t screenId,
        uint32_t& level) override;

    ErrCode SetVirtualScreenAsDefault(
        uint64_t screenId,
        bool& isSucc) override;

private:
    DisplayManagerIpcService();

    static inline SingletonDelegator<DisplayManagerIpcService> delegator_;
    DisplayManagerService& displayManagerService_;
};
} // namespace OHOS::Rosen
#endif // OHOS_ROSEN_DISPLAY_MANAGER_IPC_SERVICE_H
