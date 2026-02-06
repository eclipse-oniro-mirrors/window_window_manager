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

#ifndef OHOS_ROSEN_DMS_TEST_BASE_H
#define OHOS_ROSEN_DMS_TEST_BASE_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "rs_mock_impl.h"
#include "zidl/screen_session_manager_client_interface.h"
#include "zidl/idisplay_manager_agent.h"

#define private public
#define protected public
#include "screen_session_manager.h"
#undef private
#undef protected

namespace OHOS {
namespace Rosen {

class ScreenSessionManagerClientMock : public IRemoteStub<IScreenSessionManagerClient> {
public:
    MOCK_METHOD(void, SwitchUserCallback, (std::vector<int32_t> oldScbPids, int32_t currentScbPid), (override));
    MOCK_METHOD(void, OnScreenConnectionChanged, (SessionOption option, ScreenEvent screenEvent), (override));
    MOCK_METHOD(void, OnPropertyChanged,
                (ScreenId screenId, const ScreenProperty& property, ScreenPropertyChangeReason reason), (override));
    MOCK_METHOD(bool, OnFoldPropertyChange,
                (ScreenId screenId, const ScreenProperty& property, ScreenPropertyChangeReason reason,
                 FoldDisplayMode displayMode, ScreenProperty& midProperty),
                (override));
    MOCK_METHOD(void, OnPowerStatusChanged,
                (DisplayPowerEvent event, EventStatus status, PowerStateChangeReason reason), (override));
    MOCK_METHOD(void, OnSensorRotationChanged, (ScreenId screenId, float sensorRotation, bool isSwitchUser),
                (override));
    MOCK_METHOD(void, OnHoverStatusChanged, (ScreenId screenId, int32_t hoverStatus, bool needRotate), (override));
    MOCK_METHOD(void, OnScreenOrientationChanged, (ScreenId screenId, float screenOrientation), (override));
    MOCK_METHOD(void, OnScreenRotationLockedChanged, (ScreenId screenId, bool isLocked), (override));
    MOCK_METHOD(void, OnScreenExtendChanged, (ScreenId mainScreenId, ScreenId extendScreenId), (override));
    MOCK_METHOD(void, OnSuperFoldStatusChanged, (ScreenId screenId, SuperFoldStatus superFoldStatus), (override));
    MOCK_METHOD(void, OnSecondaryReflexionChanged, (ScreenId screenId, bool isSecondaryReflexion), (override));
    MOCK_METHOD(void, OnBeforeScreenPropertyChanged, (FoldStatus foldStatus), (override));
    MOCK_METHOD(void, OnDisplayStateChanged,
                (DisplayId defaultDisplayId, sptr<DisplayInfo> displayInfo,
                 (const std::map<DisplayId, sptr<DisplayInfo>>&)displayInfoMap, DisplayStateChangeType type),
                (override));
    MOCK_METHOD(void, OnScreenshot, (DisplayId displayId), (override));
    MOCK_METHOD(void, OnImmersiveStateChanged, (ScreenId screenId, bool& immersive), (override));
    MOCK_METHOD(void, SetDisplayNodeScreenId, (ScreenId screenId, ScreenId displayNodeScreenId), (override));
    MOCK_METHOD(void, OnGetSurfaceNodeIdsFromMissionIdsChanged,
                (std::vector<uint64_t> & missionIds, std::vector<uint64_t>& surfaceNodeIds,
                 const std::vector<uint32_t>& needWindowTypeList, bool isNeedForceCheck),
                (override));
    MOCK_METHOD(void, OnSetSurfaceNodeIdsChanged, (DisplayId displayId, const std::vector<uint64_t>& surfaceNodeIds),
                (override));
    MOCK_METHOD(void, OnVirtualScreenDisconnected, (DisplayId displayId), (override));
    MOCK_METHOD(void, OnUpdateFoldDisplayMode, (FoldDisplayMode displayMode), (override));
    MOCK_METHOD(void, SetVirtualPixelRatioSystem, (ScreenId screenId, float virtualPixelRatio), (override));
    MOCK_METHOD(void, OnFoldStatusChangedReportUE, (const std::vector<std::string>& screenFoldInfo), (override));
    MOCK_METHOD(void, ScreenCaptureNotify, (ScreenId mainScreenId, int32_t uid, const std::string& clientName),
                (override));
    MOCK_METHOD(void, OnCameraBackSelfieChanged, (ScreenId screenId, bool isCameraBackSelfie), (override));
    MOCK_METHOD(void, OnExtendScreenConnectStatusChanged,
                (ScreenId screenId, ExtendScreenConnectStatus extendScreenConnectStatus), (override));
    MOCK_METHOD(bool, OnExtendDisplayNodeChange, (ScreenId mainScreenId, ScreenId extendScreenId), (override));
    MOCK_METHOD(bool, OnCreateScreenSessionOnly,
                (ScreenId screenId, ScreenId rsId, const std::string& name, bool isExtend), (override));
    MOCK_METHOD(bool, OnMainDisplayNodeChange, (ScreenId mainScreenId, ScreenId extendScreenId, ScreenId extendRSId),
                (override));
    MOCK_METHOD(void, SetScreenCombination,
                (ScreenId mainScreenId, ScreenId extendScreenId, ScreenCombination extendCombination), (override));
    MOCK_METHOD(std::string, OnDumperClientScreenSessions, (), (override));
    MOCK_METHOD(void, OnScreenModeChanged, (ScreenModeChangeEvent screenModeChangeEvent), (override));
    MOCK_METHOD(void, OnAnimationFinish, (), (override));
    MOCK_METHOD(void, SetInternalClipToBounds, (ScreenId screenId, bool clipToBounds), (override));
    MOCK_METHOD(void, OnTentModeChange, (TentMode tentMode), (override));
};

class DisplayManagerAgentMock : public IRemoteStub<IDisplayManagerAgent> {
public:
    MOCK_METHOD(void, NotifyDisplayPowerEvent, (DisplayPowerEvent event, EventStatus status), (override));
    MOCK_METHOD(void, NotifyDisplayStateChanged, (DisplayId id, DisplayState state), (override));
    MOCK_METHOD(void, OnScreenConnect, (sptr<ScreenInfo> screenInfo), (override));
    MOCK_METHOD(void, OnScreenDisconnect, (ScreenId screenId), (override));
    MOCK_METHOD(void, OnScreenChange, (const sptr<ScreenInfo>& screenInfo, ScreenChangeEvent event), (override));
    MOCK_METHOD(void, OnScreenGroupChange,
                (const std::string& trigger, const std::vector<sptr<ScreenInfo>>& info, ScreenGroupChangeEvent event),
                (override));
    MOCK_METHOD(void, OnDisplayCreate, (sptr<DisplayInfo> info), (override));
    MOCK_METHOD(void, OnDisplayDestroy, (DisplayId displayId), (override));
    MOCK_METHOD(void, OnDisplayChange, (sptr<DisplayInfo> info, DisplayChangeEvent event), (override));
    MOCK_METHOD(void, OnScreenshot, (sptr<ScreenshotInfo> info), (override));
    MOCK_METHOD(void, NotifyPrivateWindowStateChanged, (bool hasPrivate), (override));
    MOCK_METHOD(void, NotifyPrivateStateWindowListChanged, (DisplayId id, std::vector<std::string> privacyWindowList),
                (override));
    MOCK_METHOD(void, NotifyFoldStatusChanged, (FoldStatus status), (override));
    MOCK_METHOD(void, NotifyFoldAngleChanged, (std::vector<float> foldAngles), (override));
    MOCK_METHOD(void, NotifyCaptureStatusChanged, (bool isCapture), (override));
    MOCK_METHOD(void, NotifyDisplayChangeInfoChanged, (const sptr<DisplayChangeInfo>& info), (override));
    MOCK_METHOD(void, NotifyDisplayModeChanged, (FoldDisplayMode mode), (override));
    MOCK_METHOD(void, NotifyAvailableAreaChanged, (DMRect rect, DisplayId displayId), (override));
    MOCK_METHOD(void, NotifyScreenMagneticStateChanged, (bool isMagneticState), (override));
    MOCK_METHOD(void, NotifyScreenModeChange, (const std::vector<sptr<ScreenInfo>>& screenInfos), (override));
    MOCK_METHOD(void, NotifyAbnormalScreenConnectChange, (ScreenId screenId), (override));
    MOCK_METHOD(void, NotifyBrightnessInfoChanged, (ScreenId screenId, const ScreenBrightnessInfo& info), (override));
    MOCK_METHOD(void, OnDisplayAttributeChange,
                (sptr<DisplayInfo> displayInfo, const std::vector<std::string>& attributes), (override));
};

class DmsTestBase : public testing::Test {
public:
    DmsTestBase();
    void TriggerScreenChange(ScreenId screenId, ScreenEvent screenEvent, ScreenChangeReason reason);
    void TriggerScreenConnect(ScreenId screenId);
    void TriggerScreenDisconnect(ScreenId screenId);
    bool WaitScreenConnectFinish(ScreenId screenId);
    bool WaitScreenDisconnectFinish(ScreenId screenId);

    bool CreatScreen(ScreenId screenId);
};

}  // namespace Rosen
}  // namespace OHOS
#endif