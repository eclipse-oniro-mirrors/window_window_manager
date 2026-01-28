/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ROSEN_SCREEN_SESSION_MANAGER_CLIENT_INTERFACE_H
#define OHOS_ROSEN_SCREEN_SESSION_MANAGER_CLIENT_INTERFACE_H

#include <iremote_broker.h>
#include <screen_manager/screen_types.h>

#include "display_info.h"
#include "session/screen/include/screen_property.h"

namespace OHOS::Rosen {
class IScreenSessionManagerClient : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.IScreenSessionManagerClient");

    enum class ScreenSessionManagerClientMessage : uint32_t {
        TRANS_ID_ON_SCREEN_CONNECTION_CHANGED,
        TRANS_ID_ON_PROPERTY_CHANGED,
        TRANS_ID_ON_POWER_STATUS_CHANGED,
        TRANS_ID_ON_SENSOR_ROTATION_CHANGED,
        TRANS_ID_ON_SCREEN_ORIENTATION_CHANGED,
        TRANS_ID_ON_SCREEN_ROTATION_LOCKED_CHANGED,
        TRANS_ID_ON_DISPLAY_STATE_CHANGED,
        TRANS_ID_ON_SCREEN_SHOT,
        TRANS_ID_ON_IMMERSIVE_STATE_CHANGED,
        TRANS_ID_SET_DISPLAY_NODE_SCREEN_ID,
        TRANS_ID_GET_SURFACENODEID_FROM_MISSIONID,
        TRANS_ID_SET_SURFACENODEIDS,
        TRANS_ID_ON_VIRTUAL_SCREEN_DISCONNECTED,
        TRANS_ID_SET_FOLD_DISPLAY_MODE,
        TRANS_ID_ON_SWITCH_USER_CMD,
        TRANS_ID_SET_VIRTUAL_PIXEL_RATIO_SYSTEM,
        TRANS_ID_ON_FOLDSTATUS_CHANGED_REPORT_UE,
        TRANS_ID_ON_SCREEN_EXTEND_CHANGED,
        TRANS_ID_ON_HOVER_STATUS_CHANGED,
        TRANS_ID_ON_SCREEN_CAPTURE_NOTIFY,
        TRANS_ID_ON_SUPER_FOLD_STATUS_CHANGED,
        TRANS_ID_ON_SECONDARY_REFLEXION_CHANGED,
        TRANS_ID_ON_CAMERA_BACKSELFIE_CHANGED,
        TRANS_ID_ON_EXTEND_SCREEN_CONNECT_STATUS_CHANGED,
        TRANS_ID_ON_EXTEND_CHANGED,
        TRANS_ID_ON_CREATE_SCREEN_SESSION_ONLY,
        TRANS_ID_ON_MAIN_CHANGED,
        TRANS_ID_SET_SCREEN_COMBINATION,
        TRANS_ID_ON_DUMP_SCREEN_SESSION,
        TRANS_ID_ON_BEFORE_PROPERTY_CHANGED,
        TRANS_ID_ON_SCREEN_MODE_CHANGED,
        TRANS_ID_ON_ANIMATE_FINISH_TIMEOUT,
        TRANS_ID_ON_FOLD_PROPERTY_CHANGED,
        TRANS_ID_SET_INTERNAL_CLIPTOBOUNDS,
        TRANS_ID_ON_TENT_MODE_CHANGE,
    };

    virtual void SwitchUserCallback(std::vector<int32_t> oldScbPids, int32_t currentScbPid) = 0;
    virtual void OnScreenConnectionChanged(SessionOption option, ScreenEvent screenEvent) = 0;
    virtual void OnPropertyChanged(ScreenId screenId,
        const ScreenProperty& property, ScreenPropertyChangeReason reason) = 0;
    virtual bool OnFoldPropertyChange(ScreenId screenId, const ScreenProperty& property,
            ScreenPropertyChangeReason reason, FoldDisplayMode displayMode, ScreenProperty& midProperty) = 0;
    virtual void OnPowerStatusChanged(DisplayPowerEvent event, EventStatus status,
        PowerStateChangeReason reason) = 0;
    virtual void OnSensorRotationChanged(ScreenId screenId, float sensorRotation, bool isSwitchUser) = 0;
    virtual void OnHoverStatusChanged(ScreenId screenId, int32_t hoverStatus, bool needRotate = true) = 0;
    virtual void OnScreenOrientationChanged(ScreenId screenId, float screenOrientation) = 0;
    virtual void OnScreenRotationLockedChanged(ScreenId screenId, bool isLocked) = 0;
    virtual void OnScreenExtendChanged(ScreenId mainScreenId, ScreenId extendScreenId) = 0;
    virtual void OnSuperFoldStatusChanged(ScreenId screenId, SuperFoldStatus superFoldStatus) = 0;
    virtual void OnSecondaryReflexionChanged(ScreenId screenId, bool isSecondaryReflexion) = 0;
    virtual void OnBeforeScreenPropertyChanged(FoldStatus foldStatus) = 0;

    virtual void OnDisplayStateChanged(DisplayId defaultDisplayId, sptr<DisplayInfo> displayInfo,
        const std::map<DisplayId, sptr<DisplayInfo>>& displayInfoMap, DisplayStateChangeType type) = 0;
    virtual void OnScreenshot(DisplayId displayId) = 0;
    virtual void OnImmersiveStateChanged(ScreenId screenId, bool& immersive) = 0;
    virtual void SetDisplayNodeScreenId(ScreenId screenId, ScreenId displayNodeScreenId) = 0;
    virtual void OnGetSurfaceNodeIdsFromMissionIdsChanged(std::vector<uint64_t>& missionIds,
        std::vector<uint64_t>& surfaceNodeIds, const std::vector<uint32_t>& needWindowTypeList = {},
        bool isNeedForceCheck = false) = 0;
    virtual void OnSetSurfaceNodeIdsChanged(DisplayId displayId, const std::vector<uint64_t>& surfaceNodeIds) = 0;
    virtual void OnVirtualScreenDisconnected(DisplayId displayId) = 0;
    virtual void OnUpdateFoldDisplayMode(FoldDisplayMode displayMode) = 0;
    virtual void SetVirtualPixelRatioSystem(ScreenId screenId, float virtualPixelRatio) = 0;
    virtual void OnFoldStatusChangedReportUE(const std::vector<std::string>& screenFoldInfo) = 0;
    virtual void ScreenCaptureNotify(ScreenId mainScreenId, int32_t uid, const std::string& clientName) = 0;
    virtual void OnCameraBackSelfieChanged(ScreenId screenId, bool isCameraBackSelfie) = 0;
    virtual void OnExtendScreenConnectStatusChanged(ScreenId screenId,
        ExtendScreenConnectStatus extendScreenConnectStatus) = 0;
    virtual bool OnExtendDisplayNodeChange(ScreenId mainScreenId, ScreenId extendScreenId) = 0;
    virtual bool OnCreateScreenSessionOnly(ScreenId screenId, ScreenId rsId,
        const std::string& name, bool isExtend) = 0;
    virtual bool OnMainDisplayNodeChange(ScreenId mainScreenId, ScreenId extendScreenId, ScreenId extendRSId) = 0;
    virtual void SetScreenCombination(ScreenId mainScreenId, ScreenId extendScreenId,
        ScreenCombination extendCombination) = 0;
    virtual std::string OnDumperClientScreenSessions() = 0;
    virtual void OnScreenModeChanged(ScreenModeChangeEvent screenModeChangeEvent) = 0;
    virtual void OnAnimationFinish() = 0;
    virtual void SetInternalClipToBounds(ScreenId screenId, bool clipToBounds) = 0;
    virtual void OnTentModeChange(TentMode tentMode) = 0;
};
} // namespace OHOS::Rosen

#endif // OHOS_ROSEN_SCREEN_SESSION_MANAGER_CLIENT_INTERFACE_H
