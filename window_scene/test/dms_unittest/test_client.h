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

#ifndef FRAMEWORKS_WM_TEST_UT_TEST_CLIENT_H
#define FRAMEWORKS_WM_TEST_UT_TEST_CLIENT_H

#include "zidl/screen_session_manager_client_interface.h"

namespace OHOS {
namespace Rosen {
class ScreenSessionManagerClientTest : public IScreenSessionManagerClient {
public:
    void SwitchUserCallback(std::vector<int32_t> oldScbPids, int32_t currentScbPid) override {};
    void OnScreenConnectionChanged(SessionOption option, ScreenEvent screenEvent) override {};
    void OnPropertyChanged(ScreenId screenId,
        const ScreenProperty& property, ScreenPropertyChangeReason reason) override {};
    bool OnFoldPropertyChange(ScreenId screenId, const ScreenProperty& property, ScreenPropertyChangeReason reason,
        FoldDisplayMode displayMode, ScreenProperty& midProperty) override {return true;}
    void OnPowerStatusChanged(DisplayPowerEvent event, EventStatus status,
        PowerStateChangeReason reason) override {};
    void OnSensorRotationChanged(ScreenId screenId, float sensorRotation, bool isSwitchUser) override {};
    void OnHoverStatusChanged(ScreenId screenId, int32_t hoverStatus, bool needRotate = true) override {};
    void OnScreenOrientationChanged(ScreenId screenId, float screenOrientation) override {};
    void OnScreenRotationLockedChanged(ScreenId screenId, bool isLocked) override {};
    void OnScreenExtendChanged(ScreenId mainScreenId, ScreenId extendScreenId) override {};

    void OnDisplayStateChanged(DisplayId defaultDisplayId, sptr<DisplayInfo> displayInfo,
        const std::map<DisplayId, sptr<DisplayInfo>>& displayInfoMap, DisplayStateChangeType type) override {};
    void OnScreenshot(DisplayId displayId) override {};
    void OnImmersiveStateChanged(ScreenId screenId, bool& immersive) override {};
    void SetDisplayNodeScreenId(ScreenId screenId, ScreenId displayNodeScreenId) override {};
    void OnGetSurfaceNodeIdsFromMissionIdsChanged(std::vector<uint64_t>& missionIds,
        std::vector<uint64_t>& surfaceNodeIds, const std::vector<uint32_t>& needWindowTypeList = {},
        bool isNeedForceCheck = false) override {}
    void OnSetSurfaceNodeIdsChanged(DisplayId displayId, const std::vector<uint64_t>& surfaceNodeIds) override {};
    void OnVirtualScreenDisconnected(DisplayId displayId) override {};
    void OnUpdateFoldDisplayMode(FoldDisplayMode displayMode) override {};
    void SetVirtualPixelRatioSystem(ScreenId screenId, float virtualPixelRatio) override {};
    void OnFoldStatusChangedReportUE(const std::vector<std::string>& screenFoldInfo) override {};
    void ScreenCaptureNotify(ScreenId mainScreenId, int32_t uid, const std::string& clientName) override {};
    void OnCameraBackSelfieChanged(ScreenId screenId, bool isCameraBackSelfie) override {}
    void OnSuperFoldStatusChanged(ScreenId screenId, SuperFoldStatus superFoldStatus) override {};
    void OnSecondaryReflexionChanged(ScreenId screenId, bool isSecondaryReflexion) override {};
    void OnBeforeScreenPropertyChanged(FoldStatus foldStatus) override {};
    void OnExtendScreenConnectStatusChanged(ScreenId screenId,
        ExtendScreenConnectStatus extendScreenConnectStatus) override {}
    bool OnExtendDisplayNodeChange(ScreenId mainScreenId, ScreenId extendScreenId) override { return true; };
    bool OnCreateScreenSessionOnly(ScreenId screenId, ScreenId rsId,
        const std::string& name, bool isExtend) override { return true; };
    bool OnMainDisplayNodeChange(ScreenId mainScreenId, ScreenId extendScreenId,
        ScreenId extendRSId) override { return true; };
    void SetScreenCombination(ScreenId mainScreenId, ScreenId extendScreenId,
        ScreenCombination extendCombination) override {};
    std::string OnDumperClientScreenSessions() override { return ""; };
    void OnScreenModeChanged(ScreenModeChangeEvent screenModeChangeEvent) override {};
    void OnAnimationFinish() override {};
    void SetInternalClipToBounds(ScreenId screenId, bool clipToBounds) override {};
    sptr<IRemoteObject> AsObject() override {return testPtr;};
    void OnTentModeChange(TentMode tentMode) override {};
    sptr<IRemoteObject> testPtr;
};
}
} // namespace OHOS
#endif // FRAMEWORKS_WM_TEST_UT_TEST_CLIENT_H